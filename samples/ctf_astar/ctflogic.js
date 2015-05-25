require("global.js")
require("behaviors.js")
require("navigation_graph.js")

Navmesh.hide()
navigation_graph.hide()

function otherColor(color) {
	return color === "red" ? "green" : "red";
}

var d = Scene.createDrawer("navigation_drawer")
function onSetup() {
	Scene.destroy(navigation_graph);
	Scene.setKnowledge("goal_red", flag_red.position);
	Scene.setKnowledge("goal_green", flag_green.position);

	var a = spawnFighter("red", "redAgent");
	a.enableAnimation("my_animation");

	// Draw the graph (This shows you how to use the __graph__ variable).
	for(var i = 0; i < graph.length; ++i) {
		var node = graph[i]
		d.drawCircle(node.position, 0.1, 4, Colors.GREEN, true)

		for(var j = 0; j < node.edges.length; ++j)
		{
			var edge = node.edges[j]
			var connectedNode = graph[edge.targetIndex]
			d.drawLine(node.position, connectedNode.position, Colors.RED);
		}
	}

	// This is how to tell an actor to move to a random node on the graph.
	var setNextTargetNode = function() {
		var nextRandomNode = Random.uniformInt(0, graph.length-1);

		// You should compute a sequence of nodes to get to that position using A* instead.
		// Then you should set the sequence of movement_targets appropiately.
		// (You could also draw your computed path to the scene using the Drawer API)
		a.setKnowledge("movement_target", graph[nextRandomNode].position);
	}
	setNextTargetNode();

	// This is the event that gets broadcast when the actor has reached the target position.
	// At this point you want to set the next target position on your sequence of nodes to the actual target.
	// (Right now we just choose another random node, ignoring any obstacles)
	a.knowledgeRemoved.connect(function(key) {
		if(key === "movement_target") {
			setNextTargetNode();
		}
	})
}

function update(deltaTime) {
	;
}