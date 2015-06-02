require("bt_eval.js")

var SEQ = '[S';
var SEL = '[L';

function mergeState(s1, s2) {
	for(var key in s2) {
		s1[key] = s2[key]
	}
	return s1;
}

var nodePoolIndex = 0;
var nodePool = [new Node(), new Node()]
function nextNodeFromPool() {
	if(nodePoolIndex >= nodePool.length) {

		if(nodePool.length * 2 > 2048)
			return null

		// Double the length when out of memory.
		var oldLen = nodePool.length;
		for(var i = 0; i < oldLen; ++i)
			nodePool.push(new Node())
	}

	Node.apply(nodePool[nodePoolIndex], arguments)
	return nodePool[nodePoolIndex++]
}

function resetNodePool() {
	nodePoolIndex = 0;
}

function treeFromString(string, idx, actions) {
	if(string === undefined)
		throw "Cant build tree from undefined string"

	if(idx === undefined)
		idx = 0;

	if(idx === 0)
		resetNodePool()

	if(!isInner(string[idx]))
		throw "First character in tree string must be an inner node. [" + idx + "]";

	var root = nextNodeFromPool(string[idx].indexOf(SEQ) === 0 ? "Sequence" : "Selector", [], {});
	var len = innerCount(string[idx]);

	//print("==== BEGIN ====")
	//print("PRO " + string + " (idx=" + idx + ", len=" + len + ")")

	var jumped = 0;
    for(var i = idx + 1; len != 0; ++i, --len) {
    	var c = string[i];
    	//print("CUR " + c + " (i=" + i + ", len=" + len + ")")
    	if(isInner(c)) {
    		var ic = innerCount(c)
    		//print("INN " + ic);
    		var subtree = treeFromString(string, i, actions)
    		root.children.push(subtree.root)
    		//print("PLU " + subtree.jumped)
    		jumped += (ic + subtree.jumped)
    		i += (ic + subtree.jumped)
    	} else if(isCondition(c)) {
    		root.pcs = mergeState(root.pcs, c);
    		//print("PCS " + getKeys(root.pcs));
    	} else /* isAction */ {
    		var action = actions[c];

    		if(action === undefined)
    			// FIXME: throw "Action not found " + c
    		{
    			for(var a in actions) {
    				action = a;
    				print("errooor")
    				break;
    			}
    		}
    		var nextNode = nextNodeFromPool("Action", action);
    		root.children.push(nextNode)
    		//print("ACT " + nextNode.action.name)
    	}
    }
    //print("==== END ====")
    return {"root": root, "jumped": jumped}
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
	return node = node[0] + node[1] + count;
}

function findParent(root, idx) {
	var parentStack = []
	var topStack = function() {

		return parentStack.length == 0 ? -1 : parentStack[parentStack.length - 1]
	}
	var popStack = function() {
		parentStack.splice(-1,1)
	}
	for(var i = 0; i < root.length; ++i, --topStack().len) {
		if(i == idx) {
			return topStack().idx;
		}

		if(topStack().len === 0) {
			popStack();
		}

		if(isInner(root[i])) {
			parentStack.push({"idx": i, "len:": innerCount(root[i])});
		}
	}
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
	return root.length;
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

function findBestTree(ngrams, actions, pcSet, trigrams, conditions, allSolutions, goal, params) {
	var ga = new Genetic()

	var maxAttPoints = 8;
	var cache = {}
	ga.fitness = function(c) {
		if(cache[c.root] !== undefined)
			return cache[c.root]


		var root = treeFromString(c.root, 0, actions).root

		var total = 0;
		var distSum = 0;
		for(var startState in allSolutions) {
			var solution = allSolutions[startState];
			var btSolution = btPlayout(root, buildStartState(pcSet, parseInt(startState)), goal, 5);
			//var ld = (new Levenshtein(solution.join(), btSolution.join())).distance
			var ld = String.damerauLevenshteinDistance(solution.join(), btSolution.join())
			//print(solution + " vs " + btSolution + " = " + ld)
			distSum += ld;
			total++;
		}

		//print("Fitness of: " + c.root + " = " + distSum / total);
		//printTree(root)
		return (cache[c.root] = distSum / total);
	}

	ga.crossover = function(l, r) {
		// Find random attachment points for exchange
		var p1 = Random.uniformInt(0, l.attPoints - 1)
		var p2 = Random.uniformInt(0, r.attPoints - 1)

		var idx1 = nthAttPoint(l.root, p1);
		var ic1  = innerCount(l.root[idx1]);
		//var ip1 = Random.uniformInt(0, ic1);
		var end1 = findInsertionPoint(l.root, idx1, ic1)
		//print("CRS " + l.root + " [start=" + idx1 + ", end=" + end1 + ", count=" + ic1 + "]");

		// Copy left tree
		var newString = l.root.slice()

		// strip the old subtree
		newString.splice(idx1, end1-idx1)

		var idx2 = nthAttPoint(r.root, p2);
		var ic2  = innerCount(r.root[idx2]);
		//var ip2 = Random.uniformInt(0, ic2);
		var end2 = findInsertionPoint(r.root, idx2, ic2)
		//print("AND " + r.root + " [start=" + idx2 + ", end=" + end2 + ", count=" + ic2 + "]");

		// insert new subtree
		flatInsert(newString, r.root.slice(idx2, end2), idx1)
		//print("____" + newString)

		var numAttPoints = countAttPoints(newString);
		if(numAttPoints <= maxAttPoints) {
			return {"root": newString, "attPoints": numAttPoints};
		} else {
			return l;
		}
			
	}

	ga.mutation = function(c) {
		if(Random.uniform() < 0.35) {
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
				c.root[idx] = setInnerCount(c.root[idx], ic+1)
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
		} else if(false) {
			if(c.attPoints == 1)
				return c

			// Remove a random attachment point (except the root node.)
			var p = Random.uniformInt(1, c.attPoints - 1)

			var idx = nthAttPoint(c.root, p)
			var ic = innerCount(c.root[idx])
			var end = findInsertionPoint(c.root, idx, ic)

			//print("MUT " + c.root + " [start=" + idx + ", end=" + end + ", count=" + ic + ", " + p + "]")
			var parentIdx = findParent(c.root, idx);
			c.root[parentIdx] = setInnerCount(c.root[parentIdx], innerCount(c.root[parentIdx])-1);

			// strip the subtree
			c.root.splice(idx, end-idx)
			//print('____' + c.root)
			c.attPoints = countAttPoints(c.root)

			return c;
		}

		return ga.generator(0)
		// Transform random sequence into selector
		var p = Random.uniformInt(0, c.attPoints - 1)
		var idx = nthAttPoint(c.root, p)
		var val = c.root[idx]
		val[1] = val[1] == 'L' ? 'S' : 'L'
		return c;
	}

	// Randomly generate an initial population from the characteristic sequences.
	ga.generator = function(idx) {
		var numSeqs = Random.uniformInt(2, Math.min(ngrams.length - 1, 5));
		var attPoints = 1;
		var root = ['[L0'];
		
		for(var i = 0; i < numSeqs; ++i) {
			var nextSeq = Random.uniformInt(0, ngrams.length - 1)
			var nextAtt = Random.uniformInt(0, attPoints - 1);

			var j = nthAttPoint(root, nextAtt)
			var count = innerCount(root[j])
			root[j] = setInnerCount(root[j], count+1)

			var insertionPoint = Random.uniformInt(0, count);
			insertionPoint = findInsertionPoint(root, j, insertionPoint)

			var seq = ngrams[nextSeq].split(",");

			var newNode;
			if(Random.uniform() < 0.5) {
				newNode = '[S2'
			} else {
				newNode = '[L2'
			}

			seq.splice(0,0,newNode);

			flatInsert(root, seq, insertionPoint)

			attPoints++;
		}
		return {"root": root, "attPoints": attPoints};
	}

	ga.onGeneration = function(gen, fitness) {
		//print("GA [ " + gen + " ] fitness = " + fitness);
	}

	var res;
	if(params === undefined) {
		ga.populationSize = 150;
		res = ga.optimise(10, 0.05, 0.33, 0.2)
	} else {
		ga.populationSize = params.populationSize;
		res = ga.optimise(params.generations, params.elitism, params.crossover, params.mutation)
	}
	res.fitness = cache[res.root]
	//print(res.root)
	//print(res.attPoints)
	//print(res.fitness)
	return res;
}

