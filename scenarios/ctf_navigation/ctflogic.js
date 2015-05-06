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

	var d = Scene.createDrawer("navDrawer")
	var a = spawnFighter("red", "player_team_red_0");
	a.knowledgeChanged.connect(function(key, value) {
		if(key === "movement_target") {
			var offset = new Vector3(0, 0.25, 0);
			d.drawArrow(a.position.add(offset), value.add(offset), Colors.BLUE);
		}
	})

	a.knowledgeRemoved.connect(function(key) {
		if(key == "movement_target") {
			d.clear()
		}
	})
}

function update(deltaTime) {
}