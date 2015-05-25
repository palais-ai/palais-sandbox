require("global.js")

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
				if(state[cond.name][cond.operator](cond.value)) {
					return false
				}
			}
		}
		return true
	}
}

function Action() {
	this.precondition = Planner.prototype.defaultPrecondition
	this.postcondition = Planner.prototype.defaultPostcondition
	this.cost = Planner.prototype.defaultCost
	this.perform = function(actor) {}
}

function MoveToFlagAction(goal) {
	this.name = goal.equals(flag_red.position) ? "r" : "g";
	this.preconditions = [new Precondition("position", goal, "equals")];
	this.precondition = functionFromPreconditionSet(this.preconditions);
	this.postcondition = function(state) {
		var ownGoal = state["team_color"] === "red" ? flag_red : flag_green;
		var otherGoal = state["team_color"] === "red" ? flag_green : flag_red;

		if(goal.equals(ownGoal.position) && state["has_flag"] === true) {
			state["has_flag"] = false
			state["made_points"] = true
		} else if(goal.equals(otherGoal.position)) {
			state["has_flag"] = true
		}
		state["position"] = goal;
		return state;
	}
	this.cost = function(state) {
		var p = state["position"]
		var d = p.distanceTo(goal)
		if(state["enemy_in_range"])
			d += 50
		return d;
	}
	this.perform = function(actor) {
		print(this.name)
	}
}

function ShootAction() {
	this.name = "s"
	this.preconditions = [new Precondition("enemy_in_range", true), new Precondition("has_ammo", true)];
	this.precondition = functionFromPreconditionSet(this.preconditions);
	this.postcondition = function(state) {
		state["has_ammo"] = false

		if(state["enemy_in_range.health"] === undefined) {
			state["enemy_in_range.health"] = defaultHealth
		}
		state["enemy_in_range.health"] = state["enemy_in_range.health"] - 1

		if(state["enemy_in_range.health"] == 0) {
			state["enemy_in_range"] = false
			state["killed_enemy"] = true
		}
		return state;
	}

	this.cost = function(state) {
		return 0;
	}
	this.perform = function(actor) {
		print(this.name)
	}
}

function ReloadAction() {
	this.name = "t"
	this.preconditions = [];
	this.precondition = functionFromPreconditionSet(this.preconditions);
	this.postcondition = function(state) {
		state["has_ammo"] = true
		return state;
	}
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
    var actionSet = [new MoveToFlagAction(ownFlag), 
    				 new MoveToFlagAction(otherFlag),
    				 new ReloadAction(),
    				 new ShootAction()]

   	var preconditionSet = []
	for(var i = 0; i < actionSet.length; ++i) {
		var action = actionSet[i]
	    planner.addAction(action.precondition,
	                      action.postcondition,
	                      action.cost,
	                      action.perform, 
	                      action.name);

	    preconditionSet = preconditionSet.concat(action.preconditions)
    }

    //print(planner.findPlan({"enemy_in_range" : false, "made_points" : false, "position" : actor.position}, {"made_points" : true}, actionSet.length))
    
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
		
			var path = planner.findPlan(startState, {"made_points" : true}, actionSet.length)
			//print(i + ' / ' + j + ' -- ' + path)

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

    print('Simulated 2^' + preconditionSet.length + '*' + preconditionSet.length + ' = ' + limit * preconditionSet.length + " runs.");

    var topTri = getKeys(trigrams).sort(function(l,r) {
    	return trigrams[l] < trigrams[r]
    }).slice(0, Math.min(getKeys(trigrams).length, 4))
    print(topTri)

	var topBi  = getKeys(bigrams).sort(function(l,r) {
    	return bigrams[l] < bigrams[r]
    }).slice(0, Math.min(getKeys(bigrams).length, 5))
    print(topBi)
}