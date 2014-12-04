function onStart() {
	// Initialization code here
	scene.setKnowledge("Hello", 30);
	Cube_000.setKnowledge("Hello", 22)
}

var didPrint = false;
var pos;

function update(deltaTime) {
	// Logic here

	if(!didPrint) {
		print(scene.actors)
		print(Cube_000.knowledge["Hello"]);
		print("Hello value: " + scene.knowledge["Hello"])
		print(Cube_000.position)
		print(Cube_001.position)

		//Cube_001.position = new Vector3(1,2,3);

		print(Cube_000.position)
		print(Cube_001.position)

		//Cube_000.position.x = 20
		print(Cube_000.position.x)
		print(Cube_000.position)

		pos = Cube_000.position

		didPrint = true;
	}

	pos.x += 0.5*deltaTime
	Cube_000.position = pos
}

function shoot(actor, target) {

}