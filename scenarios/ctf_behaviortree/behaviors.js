require("global.js")
require("ctf_game.js")

function getOwnFlag(color)
{
	return color === "red" ? flag_red : flag_green;
}

function getOpponentFlag(color)
{
	return color === "red" ? flag_green : flag_red;
}

function PlayAnimation(animationName)
{
	this.animationName = animationName;
	Behavior.call(this);
}

PlayAnimation.prototype =
{
	run: function() {
		var actor = this.userData["self"];
		actor.enableAnimation("my_animation");
		this.setStatus(Status.Waiting);
	},
	terminate: function() {
		var actor = this.userData["self"];
		actor.disableAnimation("my_animation");
	}
}
extend(Behavior, PlayAnimation);

// The __MoveTo__ behavior invokes the pathfinding module to move an actor to a __goal__.
function MoveTo(goal)
{
	this.goal = goal;
	Behavior.call(this);
}

MoveTo.prototype = 
{
	run: function()
	{
		var context = this;
		Pathfinding.moveActor(this.userData["self"], this.goal, function() {
			context.notifySuccess();
		});
		this.setStatus(Status.Waiting);
	}
}
extend(Behavior, MoveTo);

// The __WalkTo__ behavior plays the walking animation while moving to a location (using the __MoveTo__ behavior).
function WalkTo(goal)
{
	Parallel.call(this, new MoveTo(goal), new PlayAnimation("my_animation"));
}
extend(Parallel, WalkTo);

function HasFlag(child, actor)
{
	// BlackboardDecorators observe (boolean) value changes in the blackboards of actors.
	BlackboardDecorator.call(this, child, actor, "has_flag");
}
extend(BlackboardDecorator, HasFlag);

function TeamHasFlag(child, actor)
{
	BlackboardDecorator.call(this, child, actor, "team_has_flag");
}
extend(BlackboardDecorator, TeamHasFlag);

function EnemyInRange(child, actor)
{
	BlackboardDecorator.call(this, child, actor, "enemy_in_range");
}
extend(BlackboardDecorator, EnemyInRange);

function Shoot()
{
	Behavior.call(this);
}

Shoot.prototype = 
{
	run: function()
	{
		var context = this;
		/*shoot(userData["self"], 
				userData["nearest_enemy"], 
				function()
				{
					context.setStatus(StatusRunning);
				})*/
		setStatus(Status.Waiting);
		// We don't need to notify the parent of success or failure here,
		// because the decorator will prevent any further shooting by resetting the tree
		// once no more targets are available.
	}
}
extend(Behavior, Shoot);

// FIXME: Replace this behavior with a decorator(isCloseToCarrier) + moveTo(carrier)
function GuardCarrier()
{
	Behavior.call(this);
}
GuardCarrier.prototype =
{
	run: function()
	{
		var carrier = userData["flag_carrier"];
		var self    = userData["self"];

		if(self.position.distanceTo(carrier.position) > 0.1)
		{
			var context = this;
			pathfinding.moveActor(this.userData["self"], 
								  this.goal, 
								  function()
								  {
									  context.setStatus(Status.Running);
								  });
		}
		else
		{
			setTimeout(500, this.runAgain);
		}
		setStatus(Status.Waiting);
		// As was the case above, the decorator will cancel this behavior once it is no longer feasible.
	}
}
extend(Behavior, GuardCarrier);

function MonitorTask(actor)
{
	this.actor = actor;
	setInterval(250, function() {
		var checkDistance = function(actor, flag) {
			return actor.position.distanceTo(flag.position) < 0.1;
		};

		var flag = actor.getKnowledge("has_flag") == true ? getOwnFlag(actor.getKnowledge("team_color")) : getOpponentFlag(actor.getKnowledge("team_color"));
		if(checkDistance(actor, flag))
		{
			actor.setKnowledge("has_flag", !actor.getKnowledge("has_flag"));
		}
	})
}

for(var prop in Array.prototype)
{
	print(prop)
}

var monitored = [];
function constructBehaviorTreeForActor(actor)
{
	var color = actor.getKnowledge("team_color");
	var root = new Selector(new HasFlag(new WalkTo(getOwnFlag(color).position), actor), 
					        new EnemyInRange(new Shoot(), actor), 
					       	new TeamHasFlag(new GuardCarrier(), actor),
					       	new WalkTo(getOpponentFlag(color).position)); // FIXME: behavior might end prematurely here..

	actor.setKnowledge("self", actor);
	actor.setKnowledge("has_flag", false);
	actor.setKnowledge("team_has_flag", false);
	actor.setKnowledge("enemy_in_range", false);
	root.setUserData(actor.knowledge);
	monitored += new MonitorTask(actor);
	return root;
}