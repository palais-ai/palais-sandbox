require("global.js")
require("approach.js")
require("bt_eval.js")

function Precondition(name, value, operator) {
	this.name = name;
	this.value = value;
	// operation: equals, could be extended to support others too
	if(operator !== undefined)
		this.operator = operator
	else
		this.operator = '=='
}

function functionFromPreconditionSet(set) {
	return function(state) {
		for(var i = 0; i < set.length; ++i) {
			var cond = set[i]

			if(cond.operator === '==') {
				if(state[cond.name] !== cond.value) {
					return false
				}
			} else {
				if(state[cond.name] === undefined)
					return false

				if(state[cond.name][cond.operator](cond.value)) {
					return false
				}
			}
		}
		return true
	}
}

function Postcondition(pres, name, value) {
	this.preconditions = pres;
	this.name = name
	this.value = value
}

function functionFromPostconditionSet(set) {
	return function(state) {
		for(var i = 0; i < set.length; ++i) {
			var pcs = set[i].preconditions;
			var isPossibleFun = functionFromPreconditionSet(pcs);

			if(isPossibleFun(state)) {
				state[set[i].name] = set[i].value
			}
		}
		return state
	}
}

function Action() {
	this.precondition = Planner.prototype.defaultPrecondition
	this.postcondition = Planner.prototype.defaultPostcondition
	this.cost = Planner.prototype.defaultCost
	this.perform = function(actor) {}
}

function MoveToFlagAction(ownColor, goal) {
	this.name = goal.equals(flag_red.position) ? "r" : "g";
	this.preconditions = [];//[new Precondition("position", goal, "equals")];
	this.precondition = functionFromPreconditionSet(this.preconditions);

	var ownGoal = ownColor === "red" ? flag_red : flag_green;
	var otherGoal = ownColor === "red" ? flag_green : flag_red;

	if(goal.equals(ownGoal.position)) {
		this.postconditions = [new Postcondition([new Precondition("has_flag", true)], "made_points", true),
							   new Postcondition([new Precondition("has_flag", true)], "has_flag", false)]
	} else {
		this.postconditions = [new Postcondition([], "has_flag", true)]
	}

	this.postconditions.push(new Postcondition([], "position", goal))
	this.postcondition = functionFromPostconditionSet(this.postconditions)
	this.cost = function(state) {
		var p = state["position"]

		var d = 0;
		if(p !== undefined) {
			d = p.distanceTo(goal)
		}

		if(state["enemy_in_range"] == true) {
			// It is very costly to move when an enemy is in range.
			d += 1000
		}
		return d / 1.;
	}
	this.perform = function(actor) {
		print(this.name)
	}
}

function ShootAction() {
	this.name = "s"
	this.preconditions = [new Precondition("enemy_in_range", true), new Precondition("has_ammo", true)];
	this.precondition = functionFromPreconditionSet(this.preconditions);
	this.postconditions = [new Postcondition([new Precondition("enemy_in_range", true)], "enemy_in_range", false),
						   new Postcondition([], "has_ammo", false)]
	this.postcondition = functionFromPostconditionSet(this.postconditions);

	this.cost = function(state) {
		return 1
	}
	this.perform = function(actor) {
		print(this.name)
	}
}

function ReloadAction() {
	this.name = "t"
	this.preconditions = [];
	this.precondition = functionFromPreconditionSet(this.preconditions);
	this.postconditions = [new Postcondition([], "has_ammo", true)]
	this.postcondition = functionFromPostconditionSet(this.postconditions)
	this.cost = function(state) {
		return 5
	}
	this.perform = function(actor) {
		print(this.name)
	}
}

// Computes a simple plan using GOAP.
function calculatePlan(actor) 
{
    var teamColor = actor.getKnowledge("team_color");
    var otherFlag = teamColor == "red" ? flag_green.position : flag_red.position;
    var ownFlag   = teamColor == "red" ? flag_red.position : flag_green.position;

    var planner = new Planner();
    var actionSet = [new MoveToFlagAction(teamColor, ownFlag), 
    				 new MoveToFlagAction(teamColor, otherFlag),
    				 new ReloadAction(),
    				 new ShootAction()]

    var allActions = {}
   	var preconditionSet = []
	for(var i = 0; i < actionSet.length; ++i) {
		var action = actionSet[i]
	    planner.addAction(action.precondition,
	                      action.postcondition,
	                      action.cost,
	                      action.perform, 
	                      action.name);

	    allActions[action.name] = action
	    preconditionSet = preconditionSet.concat(action.preconditions)
    }

	
    testApproachRandom(10);
    //testApproachRandom(0, allActions, {"made_points" : true});

    return;

    var bigrams = {}
    var trigrams = {}
    var limit = Math.pow(2, preconditionSet.length);
    for(var i = 0; i < limit; ++i) {
    	var startState = {"position": actor.position};
    	for(var j = 0; j < preconditionSet.length; ++j) {
			var mask = (1 << j);
			if ((i & mask) != 0) {
				startState[ preconditionSet[j].name ] = preconditionSet[j].value;
			}
		}

		var path = planner.findPlan(startState, {"made_points" : true}, 10)
		//print(i + ' / ' + j + ' -- ' + path)

		if(path.length == 0)
			print("No path found!");

		for(var k = 0; k + 1 < path.length; ++k) {
			var ng = path[k] + path[k+1];

			if(bigrams[ng] === undefined) {
				bigrams[ng] = 1;
			} else {
				bigrams[ng] = bigrams[ng] + 1;
			}
		}

		for(var k = 0; k + 2 < path.length; ++k) {
			var ng = path[k] + path[k+1] + path[k+2];

			if(trigrams[ng] === undefined) {
				trigrams[ng] = 1;
			} else {
				trigrams[ng] = trigrams[ng] + 1;
			}
		}
    }

    print('== BT Generator ==')
    print("-- trigrams --")
    for(var ng in trigrams) {
    	print(ng + " :: " + trigrams[ng])
    }

    print('-- bigrams --')
    for(var ng in bigrams) {
    	print(ng + " :: " + bigrams[ng])
    }

    print('Simulated 2^' + preconditionSet.length + ' = ' + limit + " runs.");

    var allStartState = {"position": actor.position};
	for(var j = 0; j < preconditionSet.length; ++j) {
		allStartState[ preconditionSet[j].name ] = preconditionSet[j].value;
	}

	var costForNgram = function(ngram) {
		var c = 0
		for(var k = 0; k < ngram.length; ++k) {
			c += allActions[ngram[k]].cost(allStartState)
		}
		//print(ngram + ', has cost: ' + c);
		return c
	}

	var sortByProbability = function(set, l, r) {
		return set[l] - set[r]
	}

	var numResults = 2;

    var topTri = getKeys(trigrams).sort(function(l,r) {
    	sortByProbability(trigrams,l,r)
    }).slice(0, Math.min(getKeys(trigrams).length, 2))
    print(topTri)

	var topBi  = getKeys(bigrams).sort(function(l,r) {
    	sortByProbability(trigrams,l,r)
    }).slice(0, Math.min(getKeys(bigrams).length, 3))
    print(topBi)

	var sortByCost = function(l,r) {
    	return costForNgram(l) - costForNgram(r)
	}

	print('before: ' + topBi)
	topBi = topBi.sort(sortByCost);
	print('after: ' + topBi)
	topTri = topTri.sort(sortByCost);

    print(' -- bis [' + topBi.length + '] --')
    print('root = selector()');
    for(var i = 0; i < topBi.length; ++i) {
    	var bigram = topBi[i];
    	print('root add sequence(' + bigram.split("").join(",") + ')')
    }
    print(' -- tris [' + topTri.length + '] --')
	print('root = selector()');
    for(var i = 0; i < topTri.length; ++i) {
    	var bigram = topTri[i];
    	print('root add sequence(' + bigram.split("").join(",") + ')')
    }
}