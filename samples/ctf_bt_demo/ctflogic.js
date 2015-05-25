require("global.js")
require("behaviors.js")

Navmesh.hide()
navigation_graph.hide()

function constructBehaviorTreeForActor(actor)
{
	var color = actor.getKnowledge("team_color");
	var root = new Selector(new HasFlag(new WalkTo(getOwnFlagPos(color)), actor),
							new WalkTo(getOpponentFlagPos(color)))

	root.setUserData(actor.knowledge);
	return root;
}

function onSetup() {
	Scene.setKnowledge("goal_red", flag_red.position)
	Scene.setKnowledge("goal_green", flag_green.position)
	
	spawnFighter("red", "redAgent")

	var redBT   = constructBehaviorTreeForActor(redAgent)
	Scheduler.enqueue(redBT)
	redAgent.removedFromScene.connect(function(){
		Scheduler.dequeue(redBT)
	})

	spawnFighter("green", "greenAgent")

	var greenBT = constructBehaviorTreeForActor(greenAgent)
	Scheduler.enqueue(greenBT)
	greenAgent.removedFromScene.connect(function(){
		Scheduler.dequeue(greenBT)
	})
}

function update(deltaTime) {
	
}