require("global.js")
require("ctf_game.js")

var global = this;
function otherColor(color) {
	return color === "red" ? "green" : "red";
}

function getOwnFlagPos(color)
{
	var red   = Scene.getKnowledge("goal_red");
	var green = Scene.getKnowledge("goal_green");
	return color === "red" ? red : green;
}

function getOpponentFlagPos(color)
{
	var red   = Scene.getKnowledge("goal_red");
	var green = Scene.getKnowledge("goal_green");
	return color === "red" ? green : red;
}

function Idle(timeInMS)
{
	this.idleTime = timeInMS;
	Behavior.call(this);
}

Idle.prototype = {
	run: function() {
		var context = this;
		this.handle = setTimeout(this.idleTime, function() {
			delete context.handle
			context.notifySuccess();
		})
		this.setStatus(Status.Waiting)
	},
	terminate: function() {
		if(typeof(this.handle) !== "undefined") {
			clearTimeout(this.handle)
		}
	}
}
extend(Behavior, Idle)

function PlayAnimation(animationName)
{
	this.animationName = animationName;
	Behavior.call(this);
}

PlayAnimation.prototype =
{
	run: function() {
		this.actorRemoved = false;
		var context = this;
		var actor = this.userData["self"];
		actor.enableAnimation(this.animationName);
		actor.removedFromScene.connect(function() {
			context.actorRemoved = true;
		})
		this.notifySuccess();
	},
	terminate: function() {
		if(this.actorRemoved) {
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
		var context = this;
		var hasRoute = Pathfinding.moveActor(this.userData["self"], this.goal, function() {
			context.notifySuccess();
		});
		if(hasRoute) {
			this.setStatus(Status.Waiting);
		} else {
			this.notifyFailure();
		}
	},
	terminate: function()
	{
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
		var context = this;
		var actor = this.userData["self"];
		var nearestName = this.userData["nearest_enemy"];
		if(Scene.hasActor(nearestName)) {
			shoot(actor, Scene.getActorByName(nearestName));
			this.handle = setTimeout(this.fireRate, function() {
				// Shoot again when possible.
				delete context.handle;
				context.setStatus(Status.Running);
			})

			this.setStatus(Status.Waiting);
		}
		// We don't need to notify the parent of success or failure here,
		// because the decorator will prevent any further shooting by resetting the tree
		// once no more targets are available.
	},
	terminate: function() 
	{
		if(typeof(this.handle) !== "undefined") {
			clearTimeout(this.handle);
			delete this.handle;
		}
	}
}
extend(Behavior, Shoot);

var rangeTickTime = 1000
function MonitorTask(actor)
{
	this.actor = actor;
	this.hasFlagHandle = setInterval(250, function() {
		if(actor.getKnowledge("team_has_flag") === true) {
			return;
		}

		var checkDistance = function(actor, flagPos) {
			return actor.position.distanceTo(flagPos) < 0.1;
		};

		var ownFlagPos = getOwnFlagPos(actor.getKnowledge("team_color"))
		var opponentFlagPos = getOpponentFlagPos(actor.getKnowledge("team_color"))
		var flagPos = actor.getKnowledge("has_flag") == true ? ownFlagPos : opponentFlagPos
		if(checkDistance(actor, flagPos)) {
			if(flagPos.equals(opponentFlagPos)) {
				capture(actor)
			} else {
				score(actor)
			}
		}
	})

	this.rangeHandle = setInterval(rangeTickTime, function() {
		var query = Scene.rangeQuery(actor.position, 3)
		var result = query.actors
		for(var i = 0; i < result.length; ++i) {
			var other = result[i]
			if(other.hasKnowledge("team_color") &&
			   other.getKnowledge("team_color") !== actor.getKnowledge("team_color")) {
				actor.setKnowledge("enemy_in_range", true)
				actor.setKnowledge("nearest_enemy", other.name)
				return
			}
		}
		actor.setKnowledge("enemy_in_range", false)
	})
}