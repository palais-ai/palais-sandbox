require("global.js")
require("behaviors.js")

Navmesh.hide()

var numCalls = 1;
var defaultHealth = 2;
function spawnFighter(startPos, teamColor, index) {
	var actor = Scene.instantiate("player_team_" + teamColor + "_" + index,
								  "Soldier2" + teamColor, 
								  startPos);
	actor.setScale(0.2);

	var lookAtPos = actor.position
	lookAtPos.y = Plane.position.y

	actor.position = lookAtPos

	lookAtPos.x *= 2
	lookAtPos.y *= 2
	lookAtPos.z *= 2
	actor.lookAt(lookAtPos);
	actor.setKnowledge("team_color", teamColor);
	actor.setKnowledge("position", actor.position);
	actor.setKnowledge("health", defaultHealth);

	var root = constructBehaviorTreeForActor(actor);
	Scheduler.enqueue(root);
	actor.removedFromScene.connect(function() {
		print("dequeueing for actor " + actor.name);
		Scheduler.dequeue(root);
	});

	/**var teamKey = "team_" + teamColor;
	var before = Scene.getKnowledge(teamKey);
	before.push(actor);
	// Add actor to the team list.
	Scene.setKnowledge(teamKey, before);*/
}

function spawnTeam(teamSize, startPos) {
	var meshSuffix = numCalls == 1 ? "red" : "green";
	var spawnDelay = 1000; // in ms
	for(var i = 0; i < teamSize; ++i) {
		setTimeout(spawnDelay*i, partial(spawnFighter, startPos, meshSuffix, i));
	}
	numCalls++;
}

var timer;
function onStart() {
	var teamSize = 5;
	Scene.setKnowledge("team_red", []);
	Scene.setKnowledge("team_green", []);
	spawnTeam(teamSize, flag_red.position)
	spawnTeam(teamSize, flag_green.position)

	Scene.setKnowledge("goal_red", flag_red.position);
	Scene.setKnowledge("goal_green", flag_green.position);

	Plane.setCastShadows(false)
}

function update(deltaTime) {
	;
}