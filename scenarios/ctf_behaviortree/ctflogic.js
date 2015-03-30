require("global.js")

function shoot(actor, target) {
	print(actor.name + " shot " + target.name + ". Bang.");
}

require("behaviors.js")

Navmesh.hide()

function printAll(b)
{
	if(b.children)
	{
		print(b.children.length)
		for(var child in b.children)
		{
			printAll(child)
		}
	}
	else
	{
		for(var prop in b)
		{
			print(prop);
		}
	}
}

var numCalls = 1;
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

	var root = constructBehaviorTreeForActor(actor);
	print("root:");
	printAll(root);

	Scheduler.enqueue(root);
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
	var teamSize = 25;
	spawnTeam(teamSize, flag_red.position)
	spawnTeam(teamSize, flag_green.position)

	print(Cube_000.position)
	cubePosition = Cube_059.position

	// Test values for the knowledge inspector.
	Scene.setKnowledge("bool", true);
	Scene.setKnowledge("int", 1);
	Scene.setKnowledge("float", 2.1);
	Scene.setKnowledge("floatarr", [2.0, 2.0, 2.0]);
	Scene.setKnowledge("string", "hi");
	Scene.setKnowledge("vec3", new Vector3(3,3,3));
	Scene.setKnowledge("vec3array", [new Vector3(3,3,3), new Vector3(3,3,3)]);

	Plane.setCastShadows(false)
}

function update(deltaTime) {
	;
}