require("global.js")
require("actions.js")

Navmesh.hide()

var numCalls = 1;
function spawnFighter(startPos, teamColor, index) {
	var actor = scene.instantiate("player_team_" + teamColor + "_" + index,
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
	actor.setKnowledge("team_color", teamColor);
	actor.setKnowledge("position", actor.position);
	var otherFlag = teamColor == "red" ? flag_green.position : flag_red.position;
	var ownFlag = teamColor == "red" ? flag_red.position : flag_green.position;

	var planner = new Planner();
	var moveToOwnFlagAction = new MoveToFlagAction(ownFlag);
	var moveToOtherFlagAction = new MoveToFlagAction(otherFlag);
	planner.addAction(moveToOwnFlagAction.precondition,
				      moveToOwnFlagAction.postcondition,
				      moveToOwnFlagAction.cost,
				      moveToOwnFlagAction.perform);

	planner.addAction(moveToOtherFlagAction.precondition,
				      moveToOtherFlagAction.postcondition,
				      moveToOtherFlagAction.cost,
				      moveToOtherFlagAction.perform);

	planner.makePlan(actor, {"made_points" : true});
	pathfinding.moveActor(actor, teamColor == "red" ? flag_green.position : flag_red.position);
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

	Plane.setCastShadows(false)
}

function update(deltaTime) {
	;
}

function shoot(actor, target) {
	;
}