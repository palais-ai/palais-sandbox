require("leventshtein.js")

function uniq(a) {
    var prims = {"boolean":{}, "number":{}, "string":{}}, objs = [];

    return a.filter(function(item) {
        var type = typeof item;
        if(type in prims)
            return prims[type].hasOwnProperty(item) ? false : (prims[type][item] = true);
        else
            return objs.indexOf(item) >= 0 ? false : objs.push(item);
    });
}

function generatePreconditions(numPCs, n) {
	var pcs = []
	for(var i = 0; i < n; ++i) {
		pcs[i] = new Precondition(Random.uniformInt(0, numPCs).toString(), true);
	}
	return uniq(pcs);
}

function generatePostconditions(numPCs, n, noArrays) {
	if(noArrays === undefined)
		noArrays = false

	var pcs = []
	for(var i = 0; i < n; ++i) {
		var pres = generatePreconditions(numPCs, Random.uniformInt(0,1))
		pcs.push(new Postcondition(pres, Random.uniformInt(0, numPCs), true))
	}
	return uniq(pcs);
}

var numActions = 0;
function RandomAction(numPCs) {
	this.name = (numActions++).toString()
	this.preconditions = generatePreconditions(numPCs, Random.uniformInt(0,3))
	this.precondition = functionFromPreconditionSet(this.preconditions)
	this.postconditions = generatePostconditions(numPCs, Random.uniformInt(2,5))
	this.postcondition = functionFromPostconditionSet(this.postconditions)
	this.costBase = Random.uniform(1, 100)
	this.costDependents = generatePreconditions(numPCs, Random.uniformInt(5,8))
	var outer = this;
	this.cost = function(state) {
		var costAdd = 0;
		for(var i = 0; i < outer.costDependents.length; ++i) {
			var dep = outer.costDependents[i]
			if(state[dep.name] === dep.value) {
				costAdd += 999
			}
		}
		return outer.costBase + costAdd;
	}
	this.perform = function() {}
}

function uniqName(set) {
	var seen = []
	return set.filter(function(item) {
		return seen.indexOf(item.name) >= 0 ? false : seen.push(item.name);
	})
}

function buildStartState(pcSet, combination) {
	var startState = {}
	for(var j = 0; j < pcSet.length; ++j) {
		var mask = (1 << j);
		if ((combination & mask) != 0) {
			startState[ pcSet[j].name ] = pcSet[j].value;
		}
	}
	return startState 
};

function testApproachRandom(numActions, actionSet, goal) {
	var planner = new Planner();

   	var pcSet = []
   	var eSet = []

   	var totalPosts = 0;
   	var totalPres = 0;
   	var totalDeps = 0;
   	if(actionSet === undefined) {
	   	actionSet = {}
		for(var i = 0; i < numActions; ++i) {
			var action = new RandomAction(numActions);
			actionSet[action.name] = action;
			totalPosts += action.postconditions.length;
			for(var j = 0; j < action.postconditions.length; ++j) {
				var pc = action.postconditions[j];
				eSet.push(new Precondition(pc.name, pc.value))
				pcSet = pcSet.concat(pc.preconditions);
			}
			pcSet = pcSet.concat(action.preconditions);
			totalPres += action.preconditions.length;
			totalDeps += action.costDependents.length;
		}
	} else {
		for(var name in actionSet) {
			var action = actionSet[name]
			for(var j = 0; j < action.postconditions.length; ++j) {
				var pc = action.postconditions[j];
				eSet.push(new Precondition(pc.name, pc.value))
				pcSet = pcSet.concat(pc.preconditions);
			}
			pcSet = pcSet.concat(action.preconditions);
		}
	}

	eSet = uniqName(eSet);
	pcSet = uniqName(pcSet);

	print("-- problem structure --")
	print("num e's: " + eSet.length)
	print("num pc's: " + pcSet.length)
	print("avg. e's: " + totalPosts / numActions);
	print("avg. pc's: " + totalPres / numActions);
	print("avg. dep's: " + totalDeps / numActions);


	/*
	print("--- PRECONDITIONS --")
	for(var i = 0; i < pcSet.length; ++i) {
		print(pcSet[i].name);
	}
	print("----------")*/

	if(goal === undefined) {
		goal = {}
		var numGoalStates = 7;
		for(var i = 0; i < numGoalStates; ++i) {
			var choice = Random.uniformInt(0, eSet.length - 1);
			var e = eSet[choice];
			goal[e.name] = e.value;
		}
	} 

	for(var actionName in actionSet) {
		var action = actionSet[actionName]
		planner.addAction(action.precondition,
	                      action.postcondition,
	                      action.cost,
	                      action.perform, 
	                      action.name);
	}

	var getTopNgrams = function(ngrams, limit) {
		var sortByProbability = function(set, l, r) {
			return set[r] - set[l]
		}

	    var keys = getKeys(ngrams).sort(function(l,r) {
	    	return sortByProbability(ngrams,l,r)
	    });

	    if(limit !== undefined) {
	    	keys = keys.slice(0, Math.min(getKeys(ngrams).length, limit));
	    }

	    var retVal = {}
	    for(var i = 0; i < keys.length; ++i) {
	    	retVal[keys[i]] = ngrams[keys[i]];
	    }
	    return retVal;
	}

    var conditions = {}
	var ngramsForPath = function(path, n, ngrams, nodes) {
		for(var j = 0; j + n - 1 < path.length; ++j) {
			var ng = [path[j], path[j+1]];

			for(var o = 2; o < n; ++o) {
				ng.push(path[j+o]);
			}
			ng = ng.join(",")

			if(ngrams[ng] === undefined) {
				ngrams[ng] = 1;
			} else {
				ngrams[ng] = ngrams[ng] + 1;
			}

			if(conditions[ng] === undefined) {
				conditions[ng] = [nodes[j]]
			} else {
				conditions[ng].push(nodes[j])
			}
		}

		return ngrams;
	}

	var trigrams = {}
	var allSolutions = {}
    var extractNgrams = function(preconditionSet, k, n, numCharacteristics) {
    	if(k === undefined) {
    		k = Math.pow(2, preconditionSet.length)
    	}

    	var doMonteCarlo = false
    	var maxCombinations = Math.pow(2, preconditionSet.length);
    	var limit = maxCombinations;
    	if(k < limit) {
    		limit = k;
    		doMonteCarlo = true;
    	}
    	
    	print('Simulating with ' + limit + ' (max = ' + maxCombinations + ') steps.')

    	var ngrams = {}
    	for(var i = 0; i < limit; ++i) {
    		var nextCombination = i;
    		if(doMonteCarlo) {
    			nextCombination = Random.uniformInt(0, maxCombinations)
    		}

    		var s = buildStartState(pcSet, nextCombination)
    		var path = planner.findPlan(s, goal, 7)
    		var nodes = planner.findPlanNodes(s,goal,7)
    		allSolutions[nextCombination.toString()] = path;

    		if(path.length == 0)
				print("No path found!");

			ngrams = ngramsForPath(path, n, ngrams, nodes)
			trigrams = ngramsForPath(path, 3, trigrams, nodes)
    	}

    	return getTopNgrams(ngrams, numCharacteristics);
    };
/*
    var numSimulations = 1024
	ngrams = extractNgrams(pcSet, numSimulations, 2, 5);

	    print(" ------- NGRAMS(" + numSimulations + ") ------- ");
	    for(var ng in ngrams) {
	    	print(ng + ";" + ngrams[ng]);
	    }
	    print(" ---- END NGRAMS(" + numSimulations + ") ------- ");

	var root = buildTree(ngrams, actionSet, pcSet)
	printTree(root)
	print(btPlayout(root, {"enemy_in_range" : true}, {"made_points": true}, 7));
    */
    var maxSimulations = 200;
    var steps = 1;
    var allNgrams = {}
    for(var i = 1; i <= steps; ++i) {
    	var numSimulations = i * (maxSimulations/steps)
	    var ngLength = 2;
	    var numCharacteristics = undefined;
	    var ngrams = extractNgrams(pcSet, numSimulations, ngLength, numCharacteristics);
	    allNgrams[i.toString()] = ngrams;
	}

	var reference = allNgrams[steps.toString()]
	for(i = steps-1; i > 0; --i) {
		var current = allNgrams[i.toString()]

		for(var key in reference) {
			if(current[key] === undefined) {
				current[key] = 0;
			}
		}
	}

	var calculateOverlap = function(ngrams, solution) {
		var missed = 0;
		for(var i = 0; i < solution.length; ) {
			if(i+1 >= solution.length) {
				var keys = getKeys(ngrams);
				var hasAnyWithStarting = false;
				for(var j = 0; j < keys.length; ++j) {
					if(keys[j].indexOf(solution[i]) == 0) {
						hasAnyWithStarting = true;
						break;
					}
				}
				if(!hasAnyWithStarting)
					missed++;
				break;
			}

			if([ngrams[solution[i],solution[i + 1]]].join(",") !== undefined) {
				i+=2;
			} else {
				i++;
				missed++;
			}
		}
		return 1. - missed / (1.*solution.length);
	}

	conditions = consolidateConditions(conditions);

	var genRandSolution = function(len) {
		var actions = []
		var names = getKeys(actionSet);
		var newLen = Random.uniformInt(0, len)
		for(var i = 0; i < newLen; ++i)
			actions.push(actionSet[names[Random.uniformInt(0, names.length-1)]].name)

		return actions;
	}

	var steps = getKeys(ngrams).length;
	var idx = 1;
	var randomSolutions = []
	for(var i = 5; i <= 5; ++i) {
		var ngrams = getTopNgrams(allNgrams[idx.toString()], i);

		var root = buildTree(ngrams, actionSet, pcSet, trigrams, conditions, allSolutions, goal);

		/*var total = 0;
		var distSum = 0;
		var dist2Sum = 0;
		var theoreticalBound = 0;
		for(var startState in allSolutions) {
			var solution = allSolutions[startState];
			var btSolution = btPlayout(root, buildStartState(pcSet, parseInt(startState)), goal, 5);
			var randomSolution = genRandSolution(5);
			var dist = (new Levenshtein(solution.join(), btSolution.join())).distance;
			var dist2 = (new Levenshtein(solution.join(), randomSolution.join())).distance;
			var overlap = calculateOverlap(ngrams, solution);
			print(solution + " vs. " + btSolution + " = " + dist + " (overlap = " + overlap + " )");
			distSum += dist;
			dist2Sum += dist2;
			total++;
			theoreticalBound += overlap;
		}*/
		//print(distSum)
		//print(total)
		//print("--- run (" + i + ") ---")
		//print('Average leventshtein distance: ' + distSum / total)
		//print('Theoretical coverage bound: ' + theoreticalBound / total)
		print(i + ";" + root.fitness)
	}

	return;
	for(var i = 1; i <= steps; ++i) {
		var ngrams = allNgrams[i.toString()];

		print(" ------- NGRAMS(" + i + ") ------- ");
	    for(var ng in ngrams) {
	    	print(ng + ";" + ngrams[ng]);
	    }
	    print(" ---- END NGRAMS(" + i + ") ------- ");
	}
}