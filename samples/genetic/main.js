var actions = ['s', 't', 'u', 'v']
var ngrams = [['[S2','s','t'], ['[S2','t', 'u'], ['[S2','u','v'], ['[S2','t','s']]
var solutions = ['stuv', 'tu', 'tuv', 'stu', 'tsu', 'vtu', 'suv']
var pcs = [{"name": "test", "value": true}]

var SEQ = '[S';
var SEL = '[L';

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

function onSetup() {
	var ga = new Genetic();

	ga.populationSize = 20;

	ga.fitness = function(c) {
		return c.attPoints;
	}

	ga.crossover = function(l, r) {
		// Find random attachment points to exchange
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
		// Mutate the condition(s) of a SEQ/SEL randomly
		//print('root:' + c.root)
		//print('aps: ' + c.attPoints)
		var p = Random.uniformInt(0, c.attPoints - 1)
		//print('p: ' + p)
		var idx = nthAttPoint(c.root, p)
		//print('idx: ' + idx)
		var cond;
		var ic;
		//print('node:' + c.root[idx]);
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
			var whichPC = Random.uniformInt(0, pcs.length - 1);
			var pc = pcs[whichPC]
			if(cond[pc.name] === undefined) {
				cond[pc.name] = pc.value
			} else {
				delete cond[pc.name]
			}
		}
		//print('cond:' + getKeys(cond));
		c.root[idx+1] = cond;
		//print(c.root)
		return c;
	}

	ga.generator = function(idx) {
		var numSeqs = Random.uniformInt(1, ngrams.length - 1);
		var attPoints = 1;
		var root = ['[L0'];

		print('numSeqs: ' + numSeqs)
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
						flatInsert(root, ngrams[nextSeq], insertionPoint)
						attPoints++;
					}
					nextAtt--;
				}
			}
		}
		print(root)
		return {"root": root, "attPoints": attPoints};
	}

	var res = ga.optimise(15, 0.1, 0.6, 0.05)
	print(res.root)
	print(res.attPoints)
}

function onTeardown() {
}

function update(deltaTime) {
}