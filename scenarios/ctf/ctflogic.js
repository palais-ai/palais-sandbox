require("global.js")
require("actions.js")

var numCalls = 1;
var fighterCount = 0;
function spawnFighter(startPos, teamColor) {
	var actor = scene.instantiate("player_team_" + teamColor + "_" + fighterCount++,
								  "Soldier2" + teamColor, 
								  startPos);
	actor.enableAnimation("my_animation");
	actor.setScale(0.2);

	var lookAtPos = actor.position
	lookAtPos.y = Plane.position.y

	actor.position = lookAtPos

	lookAtPos.x *= 2
	lookAtPos.y *= 2
	lookAtPos.z *= 2
	actor.lookAt(lookAtPos);

	pathfinding.moveActor(actor, teamColor == "red" ? flag_green.position : flag_red.position);
}

function spawnTeam(teamSize, startPos) {
	var meshSuffix = numCalls == 1 ? "red" : "green";

	var spawnDelay = 1000; // in ms
	for(var i = 0; i < teamSize; ++i) {
		setTimeout(spawnDelay*i, partial(spawnFighter, startPos, meshSuffix));
	}
	numCalls++;
}

var timer;
function onStart() {
	var teamSize = 5;
	spawnTeam(teamSize, flag_red.position)
	spawnTeam(teamSize, flag_green.position)

	print(Cube_000.position)
	cubePosition = Cube_059.position

	scene.setKnowledge("bool", true);
	scene.setKnowledge("int", 1);
	scene.setKnowledge("float", 2.1);
	scene.setKnowledge("floatarr", [2.0, 2.0, 2.0]);
	scene.setKnowledge("string", "hi");
	scene.setKnowledge("vec3", new Vector3(3,3,3));
	scene.setKnowledge("vec3array", [new Vector3(3,3,3), new Vector3(3,3,3)]);

	var planner = new Planner();

	// ACTION: MoveTo-Flag
	planner.addAction(function(state) {
		return !("cant_move" in state)
	}, function(state) {
		state["position"] = flag_red.position;
		return state;
	}, function(state) {
		if("position" in state) {
			return state["position"].distanceTo(flag_red.position)
		}
		return flag_green.position.distanceTo(flag_red.position) // Assume max distance
	});

	planner.makePlan(Cube_000, {"test" : 5, "x" : true, "yzx" : new Vector3(3,3,3)});

	Navmesh.hide()
}

function update(deltaTime) {
	;
}

function shoot(actor, target) {
	;
}