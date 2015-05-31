function btPlayout(node, state, goal, maxDepth) {
	var actions = [];
	if(!isGoal(state, goal)) {
		while(actions.length <= maxDepth &&
			  !isGoal(state,goal)) {
			var result = playoutRecursive(node, state, goal, [], maxDepth - actions.length)

			if(result.actions.length == 0) {
				break;
			}

			actions = actions.concat(result.actions)
			state = result.state
		}
	} else {
		print("goal state was already reached.")
	}
	return actions
}

var id = 0;
function Node(type, childrenOrAction, pcs) {
	this.type = type;
	if(this.type === "Selector" || this.type === "Sequence") {
		this.children = childrenOrAction
	} else {
		this.action = childrenOrAction
	}
	this.pcs = pcs;
	this.id = (id++).toString();
}

function RecursiveResult(state, flag, actions) {
	this.state = state
	this.flag = flag
	this.actions = actions
}

function isGoal(state, goal) {
	for(var p in goal) {
		if(goal[p] != state[p]) {
			return false;
		} 
	}
	return true;
}

function consolidateConditions(conditions) {
	for(var k in conditions) {
		var states = conditions[k];
		var counts = {}
		var total = 0;
		for(var i = 0; i < states.length; ++i) {
			var state = states[i]

			for(var prop in state) {
				if(counts[prop] === undefined) {
					counts[prop] = {}
				}
				
				var obj = counts[prop]
				var value = state[prop]
				if(obj[value] === undefined) {
					obj[ value ] = 1;
				} else {
					obj[ value ] += 1;
				}
				total++;
			}
		}

		var newCond = {}
		for(var key in counts) {
			for(var values in counts[key]) {
				var probability = value / total;
				if(probability >= 0.9) {
					newCond[key] = value
				}
			}
		}
		conditions[k] = newCond;
	}
	return conditions;
}

function buildTree(ngrams, actions, pcSet, trigrams, conditions, allSolutions, goal) {
	var root = new Node("Selector", [])

	var ngrams = getKeys(ngrams)
	ngrams = ngrams.sort(function(l,r) {
		var left = [r, l.split(",")[0]].join(",")
		var right = [l, r.split(",")[0]].join(",")
		var lv = trigrams[left] === undefined ? 0 : trigrams[left]
		var rv = trigrams[right] === undefined ? 0 : trigrams[right]
		return lv - rv;
	})

	var bestTree = findBestTree(ngrams, actions, pcSet, trigrams, conditions, allSolutions, goal);
	return {"tree" : treeFromString(bestTree.root, 0, actions),
			"fitness" : bestTree.fitness};
}

function printTree(node) {
	var isSelector = node.type === "Selector";
	var isSequence = node.type === "Sequence";
	if(isSequence || isSelector) {
		print(node.type)
		for(var i = 0; i < node.children.length; ++i) {
			printTree(node.children[i]);
		}
	} else {
		print(node.action.name)
	}
}

function playoutRecursive(node, state, goal, actions, maxDepth) {
	var isSelector = node.type === "Selector";
	var isSequence = node.type === "Sequence";
	if(isSequence || isSelector) {
		if((node.pcs !== undefined && !isGoal(state, node.pcs)) ||
			state["has_run_" + node.id] === true) {
			return new RecursiveResult(state, false, actions)
		}

		for(var i = 0; i < node.children.length; ++i) {
			var result = playoutRecursive(node.children[i], state, goal, actions, maxDepth)
			
			if(result.flag === 'KILL') {
				result.state["has_run_" + node.id] = true;
				return new RecursiveResult(result.state, 'KILL', result.actions)
			} else if(result.flag) {
				if(isSelector) {
					result.state["has_run_" + node.id] = true;
					return new RecursiveResult(result.state, true, result.actions)
				}
			} else {
				if(isSequence) {
					result.state["has_run_" + node.id] = true;
					return new RecursiveResult(result.state, false, result.actions)
				}
			}
			state = result.state
			actions = result.actions
		}

		if(isSelector) {
			return new RecursiveResult(state, false, actions);
		} 
		return new RecursiveResult(state, true, actions);
	} else if(node.type === "Action") {
		var retVal = node.action.precondition(state);
		if(retVal) {
			var state = node.action.postcondition(state)
			if(isGoal(state, goal)) {
				retVal = 'KILL'
			}

			actions.push(node.action.name)
			if(actions.length >= maxDepth) {
				retVal = 'KILL'
			}
		}
		return new RecursiveResult(state, retVal, actions)
	}
}