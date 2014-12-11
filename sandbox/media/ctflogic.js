var numCalls = 1;
function spawnTeam(teamSize, startPos) {
	var offset = new Vector3(0,0,0);
	var offsetFactor = numCalls == 1 ? 1 : -1;
	var meshSuffix = numCalls == 1 ? "red" : "green";

	for(var i = 0; i < teamSize; ++i) {
		var actor = scene.instantiate("player_team" + numCalls + "_" + i,
									  "Soldier2" + meshSuffix, 
									  startPos);

		actor.enableAnimation("my_animation");
		actor.setScale(0.2);

		var lookAtPos = actor.position
		lookAtPos.y = Plane.position.y

		if(i % 2) {
			offset.x += offsetFactor
		} else {
			offset.z += offsetFactor
		}

		lookAtPos.x += offset.x;
		lookAtPos.z += offset.z;

		actor.position = lookAtPos
		
		lookAtPos.x *= 2
		lookAtPos.y *= 2
		lookAtPos.z *= 2
		actor.lookAt(lookAtPos);

		actor.setKnowledge("movementTarget", new Vector3(0,0,0))
	}
	numCalls++;
}

function onStart() {
	spawnTeam(5, spawn_team1.position)
	spawnTeam(5, spawn_team2.position)
}

function updateActor(deltaTime, actor) {
	var actorSpeed = 0.5 
	if(actor.hasKnowledge("movementTarget")) {
		var target = actor.knowledge["movementTarget"]
		var current = actor.position
		var step = target.subtract(current).normalize().multiply(actorSpeed * deltaTime)

		actor.position = current.add(step)
	}
} 

function update(deltaTime) {
	var actors = scene.actors
	for(var i = 0; i < actors.length; ++i) {
		updateActor(deltaTime, actors[i]);
	}
}

function shoot(actor, target) {
	;
}