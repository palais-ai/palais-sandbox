function onSetup() {
	var actor = Scene.instantiate("myCube", "Cube", new Vector3(0,0,0));
	actor.setScale(0.1)

	var drawer = Scene.createDebugDrawer("test");

	drawer.drawArrow(actor.position, actor.position.add(new Vector3(2,0,0)), Colors.RED)

	setTimeout(2000, function() {
		//drawer.clear()
	})
}

function onTeardown() {
}

function update(deltaTime) {
}