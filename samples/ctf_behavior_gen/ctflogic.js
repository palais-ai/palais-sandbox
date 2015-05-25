require("global.js")
require("behaviors.js")

Navmesh.hide()

function otherColor(color) {
	return color === "red" ? "green" : "red";
}

var numCalls = 1;
function spawnTeam(teamSize) {
	var meshSuffix = numCalls == 1 ? "red" : "green";
	var spawnDelay = 1000; // in ms
	for(var i = 0; i < teamSize; ++i) {
		setTimeout(spawnDelay*i, partial(spawnFighter, 
			meshSuffix, "player_team_" + meshSuffix + "_" + i));
	}
	numCalls++;
}

function onSetup() {
	var teamSize = 1;
	Scene.setKnowledge("goal_red", flag_red.position);
	Scene.setKnowledge("goal_green", flag_green.position);
	spawnTeam(teamSize)
	spawnTeam(teamSize)
}

var simTime = 0;
function update(deltaTime) {
	simTime += deltaTime
}

