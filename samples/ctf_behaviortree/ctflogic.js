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

var map = [];
var numSubdivs = 40;
for(var i = 0; i < numSubdivs; ++i)
{
	map[i] = [];
	for(var j = 0; j < numSubdivs; ++j)
	{
		map[i][j] = 0;
	}
}

var stepSize = (16.0 / numSubdivs);
function indexForCoord(p)
{
	return Math.round((p + 8) / stepSize);
}

function colorForValue(t)
{
	var cutoff = 0.05
	if(t < cutoff)
	{
		return Colors.WHITE
	}
	t = t - cutoff + cutoff * t
	return new Color(t, 1-t, 0, 1);
}

var heatmapDrawer = Scene.createDrawer("heatmap");
function onSetup() {
	var teamSize = 5;
	Scene.setKnowledge("goal_red", flag_red.position);
	Scene.setKnowledge("goal_green", flag_green.position);
	spawnTeam(teamSize)
	spawnTeam(teamSize)

	setTimeout(1200000, function(){
		var maxVal = 0;
		for(var i = 0; i < numSubdivs; ++i)
		{
			for(var j = 0; j < numSubdivs; ++j)
			{
				if(map[i][j] > maxVal)
					maxVal = map[i][j]
			}
		}

		print("timeout!");
		for(var i = 0; i < numSubdivs; ++i)
		{
			for(var j = 0; j < numSubdivs; ++j)
			{
				print(i + ":" + j + " = " + map[i][j] / maxVal)
				heatmapDrawer.drawCircle(new Vector3(-8 + i*stepSize, 1.5, -8 + j*stepSize), 
					0.15, 12, colorForValue(map[i][j] / maxVal), true);
			}
		}
	})
	navigation_graph.hide();
}

function update(deltaTime) {
	for(var j = 0; j < 2; ++j)
	{
		var colors = ["red", "green"]
		for(var i = 0; i < 5; ++i)
		{
			var a = Scene.getActorByName("player_team_" + colors[j] + "_" + i);
			if(a != null)
			{
				var idxX = indexForCoord(a.position.x)
				var idxY = indexForCoord(a.position.z)

				if(map[idxX] !== undefined && map[idxX][idxY] !== undefined)
					map[idxX][idxY] += deltaTime;
			}
		}
	}
}