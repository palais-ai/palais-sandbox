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
	actor.lookAt(lookAtPos.multiply(2));
	actor.setKnowledge("team_color", teamColor);
	actor.setKnowledge("health", defaultHealth);

	var root = constructBehaviorTreeForActor(actor);
	Scheduler.enqueue(root);
	actor.removedFromScene.connect(function() {
		Scheduler.dequeue(root);
	});
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
function onSetup() {
	var teamSize = 5;
	spawnTeam(teamSize, flag_red.position)
	spawnTeam(teamSize, flag_green.position)

	Scene.setKnowledge("goal_red", flag_red.position);
	Scene.setKnowledge("goal_green", flag_green.position);

	Plane.setCastShadows(false)
}

function update(deltaTime) {
	;
}