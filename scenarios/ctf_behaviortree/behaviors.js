require("global.js")

function getOwnFlag(blackboard)
{
	return blackboard["team_color"] === "red" ? flag_red : flag_green;
}

function getOpponentFlag(blackboard)
{
	return blackboard["team_color"] === "green" ? flag_red : flag_green;
}

function MoveTo(goal)
{
	this.goal = goal;

	function run(dt)
	{
		pathfinding.moveActor(this.blackboard["self"], this.goal, this.notifySuccess);
		setStatus(Behavior.StatusWaiting);
	}
}
MoveTo.prototype = new Behavior();

function HasFlag()
{
	;
}
HasFlag.prototype = new BlackboardDecorator("has_flag");

function HasFlag()
{
	;
}
HasFlag.prototype = new BlackboardDecorator("team_has_flag");

function EnemyInRange()
{
	;
}
EnemyInRange.prototype = new BlackboardDecorator("enemy_in_range");

function behaviorTreeForActor(actor)
{
	var root = new Sequence(hasFlag, inRange, teamHasFlag, new MoveTo(getOpponentFlag().position));
	return root;
}