require("global.js")
require("ctf_game.js")

var global = this;
function getOwnFlagPos(color)
{
	var red   = Scene.getKnowledge("goal_red");
	var green = Scene.getKnowledge("goal_green");
	return color === "red"   ? red : green;
}

function getOpponentFlagPos(color)
{
	var red   = Scene.getKnowledge("goal_red");
	var green = Scene.getKnowledge("goal_green");
	return color === "red" ? green : red;
}

function PlayAnimation(animationName)
{
	this.animationName = animationName;
	Behavior.call(this);
}

PlayAnimation.prototype =
{
	run: function() {
		print('running PlayAnimation');
		this.actorRemoved = false;
		var context = this;
		var actor = this.userData["self"];
		print(actor.name)
		print("play animation for " + actor.name)
		actor.enableAnimation(this.animationName);
		actor.removedFromScene.connect(function() {
			print('setting removal flag')
			context.actorRemoved = true;
		})
		this.setStatus(Status.Waiting);
	},
	terminate: function() {
		if(this.actorRemoved)
		{
			var actor = this.userData["self"];
			actor.disableAnimation(this.animationName);
		}
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
		print('running MoveTo');
		var context = this;
		Pathfinding.moveActor(this.userData["self"], this.goal, function() {
			context.notifySuccess();
		});
		this.setStatus(Status.Waiting);
	},
	terminate: function()
	{
		print("cancel move");
		Pathfinding.cancelMove(this.userData["self"]);
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
	this.fireRate = 5000;
	Behavior.call(this);
}

Shoot.prototype = 
{
	run: function()
	{
		print('running Shoot');
		var context = this;

		print(this.userData["self"].name + ' shooting.');

		if(typeof(this.userData["nearest_enemy"]) === "undefined")
		{
			// The actor we were shooting has been removed. (=killed)
			this.notifySuccess();
		}
		else
		{
			shoot(this.userData["self"], 
				  this.userData["nearest_enemy"]);

			this.handle = setTimeout(this.fireRate, function() {
				// Shoot again when possible.
				context.setStatus(Status.Running);
				delete context.handle;
			})

			this.setStatus(Status.Waiting);
		}
		// We don't need to notify the parent of success or failure here,
		// because the decorator will prevent any further shooting by resetting the tree
		// once no more targets are available.
	},
	terminate: function() 
	{
		print("terminating SHOOT :: " + this.handle)
		if(typeof(this.handle) !== "undefined")
		{
			clearTimeout(this.handle);
		}
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
		print('running GuardCarrier');
		var carrier = getFlagOwner(this.userData["team_color"]);
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
	this.hasFlagHandle = setInterval(250, function() {
		print("has flag handle")
		if(actor.getKnowledge("team_has_flag") === true)
		{
			return;
		}

		var checkDistance = function(actor, flagPos) {
			return actor.position.distanceTo(flagPos) < 0.1;
		};

		var ownFlagPos = getOwnFlagPos(actor.getKnowledge("team_color"));
		var opponentFlagPos = getOpponentFlagPos(actor.getKnowledge("team_color"));
		var flagPos = actor.getKnowledge("has_flag") == true ? ownFlagPos : opponentFlagPos;
		if(checkDistance(actor, flagPos))
		{
			if(flagPos.equals(opponentFlagPos))
			{
				capture(actor)
			}
			else
			{
				score(actor)
			}
		}
	})

	var removalFun = function(actor) {
		print("removing nearest_enemy")
		actor.setKnowledge("enemy_in_range", false)
		actor.removeKnowledge("nearest_enemy");
	}
	this.rangeHandle = setInterval(1000, function() {
		print("range handle")
		var result = Scene.rangeQuery(actor.position, 2).actors
		for(var i = 0; i < result.length; ++i)
		{
			var other = result[i]
			if(other.hasKnowledge("team_color") &&
			   other.getKnowledge("team_color") !== actor.getKnowledge("team_color"))
			{
				actor.setKnowledge("enemy_in_range", true)

				if(actor.hasKnowledge("nearest_enemy"))
				{
					var old = actor.getKnowledge("nearest_enemy")
					old.removedFromScene.disconnect(removalFun)
				}

				actor.setKnowledge("nearest_enemy", other)
				other.removedFromScene.connect(removalFun)
				return
			}
		}
		actor.setKnowledge("enemy_in_range", false)
	})
}

function constructBehaviorTreeForActor(actor)
{
	var color = actor.getKnowledge("team_color");
	var root = new Selector(new HasFlag(new WalkTo(getOwnFlagPos(color)), actor), 
					        new EnemyInRange(new Shoot(), actor), 
					       	new TeamHasFlag(new GuardCarrier(), actor),
					       	new WalkTo(getOpponentFlagPos(color)));

	actor.setKnowledge("self", actor);
	actor.setKnowledge("has_flag", false);
	actor.setKnowledge("team_has_flag", false);
	actor.setKnowledge("enemy_in_range", false);
	var monitorTask = new MonitorTask(actor);
	actor.monitorTask = monitorTask;
	actor.removedFromScene.connect(function() {
		print('removing timers');
		clearInterval(monitorTask.hasFlagHandle)
		clearInterval(monitorTask.rangeHandle)
	});
	root.setUserData(actor.knowledge);
	return root;
}