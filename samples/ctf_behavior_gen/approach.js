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

function testApproachRandom(numActions, actionSet, goal) {
	var planner = new Planner();

   	var pcSet = []
   	var eSet = []

   	if(actionSet === undefined) {
	   	actionSet = {}
		for(var i = 0; i < numActions; ++i) {
			var action = new RandomAction(numActions);
			actionSet[action.name] = action;
			for(var j = 0; j < action.postconditions.length; ++j) {
				var pc = action.postconditions[j];
				eSet.push(new Precondition(pc.name, pc.value))
				pcSet = pcSet.concat(pc.preconditions);
			}
			pcSet = pcSet.concat(action.preconditions);
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

	print("--- PRECONDITIONS --")
	for(var i = 0; i < pcSet.length; ++i) {
		print(pcSet[i].name);
	}
	print("----------")

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

	var buildStartState = function(combination) {
		var startState = {}
    	for(var j = 0; j < pcSet.length; ++j) {
			var mask = (1 << j);
			if ((combination & mask) != 0) {
				startState[ pcSet[j].name ] = pcSet[j].value;
			}
		}
		return startState 
	};

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

    		var s = buildStartState(nextCombination)
    		var path = planner.findPlan(s, goal, 10)

    		if(path.length == 0)
				print("No path found!");

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
			}
    	}

    	var sortByProbability = function(set, l, r) {
			return set[l] - set[r]
		}

	    var keys = getKeys(ngrams).sort(function(l,r) {
	    	sortByProbability(ngrams,l,r)
	    }).slice(0, Math.min(getKeys(ngrams).length, numCharacteristics));

	    var retVal = {}
	    for(var i = 0; i < keys.length; ++i) {
	    	retVal[keys[i]] = ngrams[keys[i]];
	    }
	    return retVal;
    };

    var numSimulations = 50;
    var ngLength = 2;
    var numCharacteristics = 5;
    ngrams = extractNgrams(pcSet, numSimulations, ngLength, numCharacteristics);

    print(" ------- NGRAMS ------- ");
    for(var ng in ngrams) {
    	print(ng + " :: " + ngrams[ng]); 
    }
    print(" ------- NGRAMS ------- ");
}