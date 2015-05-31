require("bt_eval.js")

var SEQ = '[S';
var SEL = '[L';

function mergeState(s1, s2) {
	for(var key in s2) {
		s1[key] = s2[key]
	}
	return s1;
}

function treeFromString(string, idx, actions) {
	if(idx === undefined)
		idx = 0;

	if(!isInner(string[idx]))
		throw "First character in tree string must be an inner node. [" + idx + "]";

	var root = new Node(string[idx].indexOf(SEQ) === 0 ? "Sequence" : "Selector", [], {});
    for(var i = idx + 1; i < string.length; ++i) {
    	var c = string[i];

    	if(isInner(c)) {
    		root.children.push(treeFromString(string, i, actions))
    		i += innerCount(c)
    	} else if(isCondition(c)) {
    		root.pcs = mergeState(root.pcs, c);
    	} else /* isAction */ {
    		root.children.push(new Node("Action", actions[c]))
    	}
    }
    return root;
}

function flatInsert(arr1, arr2, position) {
	if(position == undefined)
		position = arr1.length

	var args = [position, 0];
	Array.prototype.push.apply(args, arr2)
	Array.prototype.splice.apply(arr1, args)
}

function isInner(node) {
	return (typeof node === "string" || node instanceof String) &&
		   (node.indexOf(SEQ) === 0 || node.indexOf(SEL) === 0);
}

function isCondition(node) {
	return node === Object(node)
}

function innerCount(node) {
	return parseInt(node.slice(2, node.length));
}

function setInnerCount(node, count) {
	return node[0] + node[1] + count;
}

function findInsertionPoint(root, current, idx) {
	for(var i = current + 1; i < root.length; ++i, --idx) {
		if(idx == 0) {
			return i;
		}

		if(isInner(root[i])) {
			idx += innerCount(root[i])
		}
	}
}

function countAttPoints(root) {
	var attPoints = 0;
	for(var i = 0; i < root.length; ++i) {
		if(isInner(root[i])) {
			attPoints += 1
		}
	}
	return attPoints;
}

function nthAttPoint(root, n) {
	for(var i = 0; i < root.length; ++i) {
		if(isInner(root[i])) {
			if(n == 0) {
				return i;
			} else {
				n--;
			}
		}
	}
	return root.length
}

function getKeys(o) {
  var result = [];
  for (var name in o) {
      if (hasOwnProperty.call(o, name))
          result.push(name);
  }
  return result
}

function findBestTree(ngrams, actions, pcSet, trigrams, conditions, allSolutions, goal) {
	var ga = new Genetic()

	var cache = {}
	ga.fitness = function(c) {
		if(cache[c.root] !== undefined)
			return cache[c.root]

		var root = treeFromString(c.root, 0, actions)
		var total = 0;
		var distSum = 0;
		for(var startState in allSolutions) {
			var solution = allSolutions[startState];
			var btSolution = btPlayout(root, buildStartState(pcSet, parseInt(startState)), goal, 5);
			distSum += (new Levenshtein(solution.join(), btSolution.join())).distance;
			total++;
		}

		return (cache[c.root] = distSum / total);
	}

	ga.crossover = function(l, r) {
		// Find random attachment points for exchange
		var p1 = Random.uniformInt(0, l.attPoints - 1)
		var p2 = Random.uniformInt(0, r.attPoints - 1)

		var idx1 = nthAttPoint(l.root, p1);
		var end1 = findInsertionPoint(l.root, idx1, innerCount(l.root))

		// Copy left tree
		var newString = l.root.slice()

		// strip the old subtree
		newString.splice(idx1, end1-idx1-1)

		var idx2 = nthAttPoint(r.root, p2);
		var end2 = findInsertionPoint(r.root, idx2, innerCount(r.root))

		// insert new subtree
		flatInsert(newString, r.root.slice(idx2, end2), idx1)
		return {"root": newString, "attPoints": countAttPoints(newString)};
	}

	ga.mutation = function(c) {
		if(Random.uniform() < 0.7)
		{
			// Mutate the condition(s) of a SEQ/SEL randomly
			var p = Random.uniformInt(0, c.attPoints - 1)
			var idx = nthAttPoint(c.root, p)
			var cond;
			var ic;
			if((ic = innerCount(c.root[idx])) != 0 && 
			    idx + 1 < c.root.length &&
			    isCondition(c.root[idx+1])) {

				// Keep old condition
			    cond = c.root[idx+1]
			} else {
				// Insert new condition
				cond = {}
				setInnerCount(c.root[idx], ic+1)
				flatInsert(c.root, [cond], idx+1);
			}

			// Adapt condition
			var numConds = Random.uniformInt(0,5)
			for(var i = 0; i < numConds; ++i) {
				var whichPC = Random.uniformInt(0, pcSet.length - 1);
				var pc = pcSet[whichPC]
				if(cond[pc.name] === undefined) {
					cond[pc.name] = pc.value
				} else {
					delete cond[pc.name]
				}
			}
			c.root[idx+1] = cond;
			return c;
		}
		else
		{
			// Remove a random attachment point
			var p = Random.uniformInt(0, c.attPoints - 1)
			var idx = nthAttPoint(c.root, p)
		}
	}

	// Randomly generate an initial population from the characteristic sequences.
	ga.generator = function(idx) {
		var numSeqs = Random.uniformInt(1, ngrams.length - 1);
		var attPoints = 1;
		var root = ['[L0'];

		for(var i = 0; i < numSeqs; ++i)
		{
			var nextSeq = Random.uniformInt(0, ngrams.length - 1)
			var nextAtt = Random.uniformInt(0, attPoints - 1);

			for(var j = 0; j < root.length; ++j) {
				if(isInner(root[j])) {
					if(nextAtt == 0) {
						var count = innerCount(root[j])
						setInnerCount(root[j], count+1)
						var insertionPoint = 1 + Random.uniformInt(0, count);
						insertionPoint = findInsertionPoint(root, j, insertionPoint)
						var seq = ngrams[nextSeq].split(",");
						seq.splice(0,0,'[S2');
						flatInsert(root, seq, insertionPoint)
						attPoints++;
					}
					nextAtt--;
				}
			}
		}
		//print(root)
		return {"root": root, "attPoints": attPoints};
	}

	ga.populationSize = 25;
	var res = ga.optimise(10, 0.1, 0.6, 0.05)
	res.fitness = ga.fitness(res)
	//print(res.root)
	//print(res.attPoints)
	//print(res.fitness)
	return res;
}

