require("global.js")
require("behaviors.js")

Navmesh.hide()

function otherColor(color) {
	return color === "red" ? "green" : "red";
}

var numCalls = 1;
var defaultHealth = 2;
function spawnFighter(teamColor, index) {
	var startPos = Scene.getKnowledge("goal_" + teamColor);
	var actor = Scene.instantiate("player_team_" + teamColor + "_" + index,
								  "Soldier2" + teamColor, 
								  startPos);

	var lookAtPos = Scene.getKnowledge("goal_" + otherColor(teamColor));
	actor.lookAt(lookAtPos);
	actor.setScale(0.2);
	actor.setKnowledge("team_color", teamColor);
	actor.setKnowledge("health", defaultHealth);
	actor.setKnowledge("movement_speed", 1)   // in meters per second.
	actor.setKnowledge("rotation_speed", 140) // in degrees per second.

	var root = constructBehaviorTreeForActor(actor);
	Scheduler.enqueue(root);
	actor.removedFromScene.connect(function() {
		Scheduler.dequeue(root);
	});
	actor.knowledgeChanged.connect(function(key, value){
		if(key === "movement_target") {
			value.y = Plane.position.y;
			actor.setKnowledge("lookat_target", value);
		}
	})
}

function spawnTeam(teamSize) {
	var meshSuffix = numCalls == 1 ? "red" : "green";
	var spawnDelay = 1000; // in ms
	for(var i = 0; i < teamSize; ++i) {
		setTimeout(spawnDelay*i, partial(spawnFighter, meshSuffix, i));
	}
	numCalls++;
}

var timer;
function onSetup() {
	var teamSize = 5;
	Scene.setKnowledge("goal_red", flag_red.position);
	Scene.setKnowledge("goal_green", flag_green.position);
	spawnTeam(teamSize)
	spawnTeam(teamSize)

	Plane.setCastShadows(false)
}

function update(deltaTime) {
	;
}