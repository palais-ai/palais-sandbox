function partial(func /*, 0..n args */) {
	var args = Array.prototype.slice.call(arguments, 1);
	return function() {
		var allArguments = args.concat(Array.prototype.slice.call(arguments));
		return func.apply(this, allArguments);
	};
}

var numCalls = 1;
var fighterCount = 0;
function spawnFighter(startPos, teamColor) {
	var actor = scene.instantiate("player_team" + numCalls + "_" + fighterCount++,
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

function testRaycast() {
	var result = scene.raycast(new Vector3(0,0,0), new Vector3(1,0,0))
	var result2 = scene.raycast(new Vector3(0,0,0), new Vector3(0,1,0))
	var result3 = scene.raycast(new Vector3(0,0,0), new Vector3(0,0,1))
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
	scene.setKnowledge("vec3array", [new Vector3(3,3,3), new Vector3(3,3,3)])

	timer = setInterval(1000, function() {
		print("test timer")
	});
	clearInterval(timer);
}

function update(deltaTime) {
	;
}

function shoot(actor, target) {
	;
}