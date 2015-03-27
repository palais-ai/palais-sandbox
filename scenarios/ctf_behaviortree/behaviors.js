require("global.js")

function getOwnFlag(userData)
{
	return userData["team_color"] === "red" ? flag_red : flag_green;
}

function getOpponentFlag(userData)
{
	return userData["team_color"] === "green" ? flag_red : flag_green;
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
		print('moving');
		pathfinding.moveActor(this.userData["self"], this.goal, this.notifySuccess);
		setStatus(StatusWaiting);
	}
}
extend(Behavior, MoveTo)

function HasFlag(child, actor)
{
	BlackboardDecorator.call(this, child, actor, "has_flag");
}
extend(BlackboardDecorator, HasFlag)

function TeamHasFlag(child, actor)
{
	BlackboardDecorator.call(this, child, actor, "team_has_flag");
}
extend(BlackboardDecorator, TeamHasFlag)

function EnemyInRange(child, actor)
{
	BlackboardDecorator.call(this, child, actor, "enemy_in_range");
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
		setStatus(StatusRunning);
	},
	run: function()
	{
		//shoot(userData["self"], userData["nearest_enemy"], onShootResult)
		setStatus(StatusWaiting);
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
		setStatus(StatusRunning);
	},
	run: function()
	{
		var carrier = userData["flag_carrier"];
		var self    = userData["self"];

		if(self.position.distanceTo(carrier.position) > 0.1)
		{
			pathfinding.moveActor(this.userData["self"], this.goal, this.runAgain);
		}
		else
		{
			setTimeout(500, this.runAgain);
		}
		setStatus(StatusWaiting);
		// As was the case above, the decorator will cancel this behavior once it is no longer feasible.
	}
}
extend(Behavior, GuardCarrier)

function constructBehaviorTreeForActor(actor)
{
	var root = new Sequence(new HasFlag(new MoveTo(getOwnFlag(actor.knowledge).position), actor), 
					        new EnemyInRange(new Shoot(), actor), 
					       	new TeamHasFlag(new GuardCarrier(), actor), 
					       	new MoveTo(getOpponentFlag(actor.knowledge).position));

	actor.setKnowledge("self", actor);
	root.setUserData(actor.knowledge);
	return root;
}