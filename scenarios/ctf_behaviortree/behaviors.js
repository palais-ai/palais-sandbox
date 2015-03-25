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
	Behavior.call(this);
}

MoveTo.prototype = 
{
	run: function()
	{
		pathfinding.moveActor(this.blackboard["self"], this.goal, this.notifySuccess);
		setStatus(Behavior.StatusWaiting);
	}
}

extend(Behavior, MoveTo)

function HasFlag(child)
{
	BlackboardDecorator.call(this, child, "has_flag");
}
extend(BlackboardDecorator, HasFlag)

function TeamHasFlag(child)
{
	BlackboardDecorator.call(this, child, "team_has_flag");
}
extend(BlackboardDecorator, TeamHasFlag)

function EnemyInRange(child)
{
	BlackboardDecorator.call(this, child, "enemy_in_range");
}
extend(BlackboardDecorator, EnemyInRange)

function Shoot()
{
	Behavior.call(this);
}

Shoot.prototype = 
{
	onShootResult: function()
	{
		setStatus(Behavior.StatusRunning);
	},
	run: function()
	{
		//shoot(blackboard["self"], blackboard["nearest_enemy"], onShootResult)
		setStatus(Behavior.StatusWaiting);
		// We don't need to notify the parent of success or failure here,
		// because the decorator will prevent any further shooting by resetting the tree
		// once no more targets are available.
	}
}
extend(Behavior, Shoot)

// FIXME: Replace this behavior with a decorator(isCloseToCarrier) + moveTo(carrier)
function GuardCarrier()
{
	Behavior.call(this);
}
GuardCarrier.prototype =
{
	runAgain: function()
	{
		setStatus(Behavior.StatusRunning);
	},
	run: function()
	{
		var carrier = blackboard["flag_carrier"];
		var self    = blackboard["self"];

		if(self.position.distanceTo(carrier.position) > 0.1)
		{
			pathfinding.moveActor(this.blackboard["self"], this.goal, this.runAgain);
		}
		else
		{
			setTimeout(500, this.runAgain);
		}
		setStatus(Behavior.StatusWaiting);
		// As was the case above, the decorator will cancel this behavior once it is no longer feasible.
	}
}
extend(Behavior, GuardCarrier)

function constructBehaviorTreeForActor(actor)
{
	var root = new Sequence(new HasFlag(new MoveTo(getOwnFlag(actor.knowledge).position)), 
					        new EnemyInRange(new Shoot()), 
					       	new TeamHasFlag(new GuardCarrier()), 
					       	new MoveTo(getOpponentFlag(actor.knowledge).position));
	return root;
}