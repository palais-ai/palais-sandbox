function btPlayout(node, state, goal, maxDepth) {
	var actions = [];
	actions = playoutRecursive(node, state, goal, actions, maxDepth)
	return actions
}

function Node(type, childrenOrAction) {
	this.type = type;
	if(this.type === "Selector" || this.type === "Sequence") {
		this.children = childrenOrAction
	} else {
		this.action = childrenOrAction
	}
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

function playoutRecursive(node, state, goal, actions, maxDepth) {
	if(isGoal(state, goal))
		return new RecursiveResult(state, 'KILL', actions);

	var isSelector = node.type === "Selector";
	var isSequence = node.type === "Sequence";
	if(isSequence || isSelector) {
		for(var i = 0; i < node.children.length; ++i) {
			var result = playoutRecursive(node.children[i], state, actions)
			
			if(result.flag === 'KILL') {
				return new RecursiveResult(result.state, 'KILL', result.actions)
			} else if(result.flag) {
				if(isSelector) {
					return new RecursiveResult(result.state, true, result.actions)
				}
			} else {
				if(isSequence) {
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