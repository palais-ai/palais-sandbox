require("movement.js")
require("navigation_graph.js")

function spawnAgent(position, color) {
	var agent = Scene.instantiate(color + "Agent", "soldier2" + color, position);
	agent.setScale(0.2)
    agent.setKnowledge("rotation_speed", 140) // in degrees per second.
    agent.enableAnimation("my_animation")
	return agent;
}

var d = Scene.createDrawer("velocityDrawer");
function onSetup() {
	spawnAgent(new Vector3(0,0,0), "green");
	spawnAgent(new Vector3(2.5,0,2.5), "red");

	redAgent.setKnowledge("lookat_target", greenAgent.position);
	redAgent.mask = 0;
	greenAgent.setKnowledge("lookat_target", redAgent.position);
	greenAgent.setKnowledge("movement_target", new Vector3(-3,0,3))
	greenAgent.knowledgeRemoved.connect(function(key) {
		// Keep moving between the two target points indefinitely.
		if(key == "movement_target") {
			var p = greenAgent.position;
			greenAgent.setKnowledge("movement_target", new Vector3(-p.x, p.y, -p.z))
		}
	})
	greenAgent.setKnowledge("movement_speed", 1)
}

function drawPath(velocity) {
		var s = redAgent.position;
		s.y += 0.2;
		var e = redAgent.position.add(velocity);
		e.y += 0.2;
		d.drawArrow(s, e, Colors.RED);
}

var accum = 0;
function update(deltaTime) {
	var maxSpeed = 0.75;
	var before = redAgent.position;

	// Apply the custom movement behavior.
	var v = movementBehavior(redAgent, greenAgent.position, maxSpeed);

	var v2 = avoidObstacle(redAgent.position, v, maxSpeed, 1);

	if(!v2.equals(new Vector3(0,0,0))) {
		v = v2;
	}

	// Clamp the agent's velocity to __maxSpeed__.
	if(v.length() > maxSpeed) {
		v.normalize().multiply(maxSpeed);
	}
	var velocity = new Vector3(v.x, v.y, v.z);
	velocity.multiply(deltaTime);

	// Draw the path fo the agent for the first 8 seconds.
	accum += deltaTime;
	if(accum < 8) {
		drawPath(velocity);
	}

	// Apply the position update if the agent isn't going to move out of bounds.
	var newPos = before.add(velocity);
	if(newPos.x < 8 && newPos.x > -8 && newPos.z < 8 && newPos.z > -8) {
		redAgent.position = newPos;
	}

	// Update the actor's orientations.
	redAgent.setKnowledge("lookat_target", greenAgent.position)
	greenAgent.setKnowledge("lookat_target", redAgent.position)
}