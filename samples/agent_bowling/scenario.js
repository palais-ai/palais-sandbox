require("actions.js")

var barrels = [];
var pins = [];
var actions = {};
var plan;
var planIdx = 0;
function performNext() {
	if(planIdx < plan.length)
		actions[plan[planIdx++]].perform(redAgent, performNext)
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
	}

    agent.setKnowledge("all_pins_done", false)
    agent.setKnowledge("barrels", barrels)
    agent.setKnowledge("pins", pins)
    planner.makePlan(agent, {"all_pins_done" : true}, pins.length*2);

    plan = agent.getKnowledge("plan");
    
    Scene.pause();
    performNext()

	return agent;
}

var d = Scene.createDrawer("velocityDrawer");
function onSetup() {
	var seed = 14;
	for(var i=0; i < seed; ++i)
	{
		Random.uniform(i, seed);
	}

	for(var i=0; i < 5; ++i)
	{	
		var barrel = Scene.instantiate("barrel_" + i, "Barrel", new Vector3(Random.uniform(-1,1), 0.02, Random.uniform(-1,1)));
		barrel.setScale(0.025)
		barrels.push(barrel);
		var c = barrel.position;
		c.y += 0.2;
		d.drawCircle(c, 5, 32, Colors.BLUE)
	}
	for(var i=0; i < 5; ++i)
	{
		var pin = Scene.instantiate("pin_" + i, "Pin", new Vector3(Random.uniform(-1,1), 0.02, Random.uniform(-1,1)));
		pin.setScale(0.05)
		pins.push(pin);
	}

	spawnAgent(new Vector3(0,0,0), "red");

	//redAgent.setKnowledge("lookat_target", greenAgent.position);
	redAgent.mask = 0;
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