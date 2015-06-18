require("actions.js")

var barrels = [];
var pins = [];
var actions = {};
var correspondingMove = {};
var actionsArray = [];
var plan;
var planIdx = 0;
function performNext() {
	if(planIdx < plan.length) {
		//print("plan[" + planIdx + "] = " + plan[planIdx].name)
		plan[planIdx++].perform(redAgent, performNext)
	}
}

function spawnAgent(position, color) {
	var agent = Scene.instantiate(color + "Agent", "soldier2" + color, position);
	agent.setScale(0.2)
    agent.setKnowledge("rotation_speed", 140) // in degrees per second.
    agent.enableAnimation("my_animation")
    agent.setKnowledge("self", agent);
    agent.setKnowledge("position", position)

    var planner = new Planner();

    for(var i = 0; i < barrels.length; ++i) {
	    var moveTo = new MoveToAction(barrels[i].position);

	    planner.addAction(moveTo.precondition,
	                      moveTo.postcondition,
	                      moveTo.cost,
	                      moveTo.perform, 
	                      moveTo.name);
	    actions[moveTo.name] = moveTo;

	    for(var j = 0; j < pins.length; ++j) {
	    	if(pins[j].position.distanceTo(barrels[i].position) > 5)
	    		continue;

	    	var th = new ThrowBarrelAction(barrels[i], pins[j])
		    planner.addAction(th.precondition,
		                      th.postcondition,
		                      th.cost,
		                      th.perform, 
		                      th.name);
		    actions[th.name] = th;
		    correspondingMove[th.name] = moveTo;
	    }
	}

	for(var i = 0; i < pins.length; ++i) {
		var moveTo = new MoveToAction(pins[i].position);

	    planner.addAction(moveTo.precondition,
	                      moveTo.postcondition,
	                      moveTo.cost,
	                      moveTo.perform, 
	                      moveTo.name);
	    actions[moveTo.name] = moveTo;

    	var th = new ThrowPinAction(pins[i])
	    planner.addAction(th.precondition,
	                      th.postcondition,
	                      th.cost,
	                      th.perform, 
	                      th.name);
	    actions[th.name] = th;
    	actionsArray.push(th)
		correspondingMove[th.name] = moveTo;
	}

    agent.setKnowledge("all_pins_done", false)
    agent.setKnowledge("barrels", barrels)
    agent.setKnowledge("pins", pins)
    //planner.makePlan(agent, {"all_pins_done" : true}, pins.length*2);

    //plan = agent.getKnowledge("plan");
    
    //Scene.pause();
    //performNext()

	return agent;
}

function checkValid(c, where) {
	if(c.length == 0) {
		print("Found invalid chromosome. At = size 0, where= " + where);
	}

	for(var i = 0; i < c.length; ++i) {
		if(!(c instanceof ThrowPinAction) &&
		   !(c instanceof ThrowBarrelAction) &&
		   !(c instanceof Object)) {
		   	print("Found invalid chromosome. At = " + i + ", where= " + where);
			Scene.pause();
			return false;
		}
	}
	return true;
}

function fitnessFunction(c) {
	var state = redAgent.knowledgeMap;
	var names = []
	var cost = 0;
	for(var i = 0; i < c.length; ++i) {
		if(names.indexOf(c[i].target.name) !== -1) {
			cost += 999;
		}
		names.push(c[i].target.name)

		var a = correspondingMove[c[i].name]
		//print(i + " = " + c[i].name);
		cost += a.cost(state);
		state = a.postcondition(state);
		//print("pinlen = " + state["pins"].length);
		//print("pos = " + state["position"])
	}
	//print("cost[" + c.length + "] = " + cost);

	return cost;
}

function performGA(agent, populationSize, generations) {
	var ga = new Genetic();

	ga.populationSize = populationSize;
	ga.fitness = fitnessFunction;

	ga.crossover = function(l, r) {
		var idx = Random.uniformInt(0, l.length - 1)

		var parent = l.slice();
		var sub = r.slice(idx, r.length);
		for(var i = 0; i < parent.length; ++i) {
			for(var j = 0; j < sub.length; ++j) {
				if(sub[j].pin.name === parent[i].pin.name)
					parent.splice(i, 1);
				else if(sub[j].target.name === parent[i].target.name)
					return l; 
			}
		}

		var c = parent.concat(sub);
		checkValid(c, "crossover");
		return c;
	}

	ga.mutation = function(c) {
		var i = Random.uniformInt(0, c.length - 1)
		var pin = c[i].pin;

		if(c[i].name.indexOf("ThrowPin_") === 0) {
			var possibleBarrels = []
		    for(var j = 0; j < barrels.length; ++j) {
		    	if(pin.position.distanceTo(barrels[j].position) > 5) {
		    		continue;
		    	}
				
			    possibleBarrels.push(barrels[j]);
		    }

		    if(possibleBarrels.length != 0) {
		    	var idx = Random.uniformInt(0, possibleBarrels.length - 1);
		    	c[i] = new ThrowBarrelAction(possibleBarrels[idx], pin);
		    }
		} else {
			c[i] = new ThrowPinAction(pin)
		}

		checkValid(c, "mutate");
		return c;
	}

	ga.generator = function(idx) {
		var c = []
		for(var i = 0; i < actionsArray.length; ++i) {
			c.push(actionsArray[i]);
		}

		// Generate solutions by shuffling the actions randomly.
		for(var i = c.length - 1; i != 0; --i) {
			var j = Random.uniformInt(0, i);
			var tmp = c[i];
			c[i] = c[j];
			c[j] = tmp;
		}

		checkValid(c, "generate");
		return c;
	}

	var res = ga.optimise(generations, 0.05, 0.5, 0.25)

	return res;
}

var d = Scene.createDrawer("velocityDrawer");
function onSetup() {
	var seed = 2;
	for(var i=0; i < seed; ++i)
	{
		Random.uniform(i, seed);
	}

	for(var i=0; i < 5; ++i)
	{	
		var barrel = Scene.instantiate("barrel_" + i, "Barrel", new Vector3(Random.uniform(-6,6), 0.02, Random.uniform(-6,6)));
		barrel.setScale(0.025)
		barrels.push(barrel);
		var c = barrel.position;
		c.y += 0.2;
		//d.drawCircle(c, 5, 32, Colors.BLUE)
	}
	for(var i=0; i < 3; ++i)
	{
		var pin = Scene.instantiate("pin_" + i, "Pin", new Vector3(Random.uniform(-8,8), 0.02, Random.uniform(-8,8)));
		pin.setScale(0.05)
		pins.push(pin);
	}

	spawnAgent(new Vector3(0,0,0), "red");
	var res = performGA(redAgent, 25, 5)
	print(fitnessFunction(res))

	//redAgent.setKnowledge("lookat_target", greenAgent.position);
	redAgent.mask = 0;

	plan = []
	for(var i = 0; i < res.length; ++i) {
		var a = correspondingMove[res[i].name]
		plan.push(a);
		plan.push(res[i]);
	}
    //print("plan size = " + plan.length)
    Scene.pause();
    performNext()
}

function drawPath() {
	var s = redAgent.position;
	s.y += 0.2;
	d.drawCircle(s, 0.02, 4, Colors.RED);
}

var accum = 0;
function update(deltaTime) {
	// Draw the path fo the agent for the first 8 seconds.
	accum += deltaTime;
	if(accum < 64) {
		drawPath();
	}
}