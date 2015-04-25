require("global.js")
require("actions.js")

var gPos = flag_green.position;
flag_red.position = new Vector3(gPos.x * -1, gPos.y, gPos.z * -1)

function setInitialState(actor) 
{  
    var defaultHealth = 2;
    actor.setKnowledge("health", defaultHealth)
}

function calculatePlan(actor) 
{
    var teamColor = actor.getKnowledge("team_color");
    var otherFlag = teamColor == "red" ? flag_green.position : flag_red.position;
    var ownFlag = teamColor == "red" ? flag_red.position : flag_green.position;

    var planner = new Planner();
    var moveToOwnFlagAction = new MoveToFlagAction(ownFlag);
    var moveToOtherFlagAction = new MoveToFlagAction(otherFlag);
    planner.addAction(moveToOwnFlagAction.precondition,
                      moveToOwnFlagAction.postcondition,
                      moveToOwnFlagAction.cost,
                      moveToOwnFlagAction.perform, "moveToOwnFlag");

    planner.addAction(moveToOtherFlagAction.precondition,
                      moveToOtherFlagAction.postcondition,
                      moveToOtherFlagAction.cost,
                      moveToOtherFlagAction.perform, "moveToOtherFlag");

    planner.makePlan(actor, {"made_points" : true});
}

function spawnFighter(teamColor, name) 
{
    var startPos = Scene.getKnowledge("goal_" + teamColor)
    var actor = Scene.instantiate(name,
                                  "Soldier2" + teamColor,
                                  startPos)

    var lookAtPos = Scene.getKnowledge("goal_" + otherColor(teamColor))
    actor.lookAt(lookAtPos)
    actor.setScale(0.2)
    setInitialState(actor)
    actor.setKnowledge("team_color", teamColor)
    actor.setKnowledge("movement_speed", 1)   // in meters per second.
    actor.setKnowledge("rotation_speed", 140) // in degrees per second.

    var root = constructBehaviorTreeForActor(actor)

    Scheduler.enqueue(root)
    actor.removedFromScene.connect(function() {
        Scheduler.dequeue(root)
    });

    actor.knowledgeChanged.connect(function(key, value) {
        if(key === "movement_target") {
            value.y = Plane.position.y
            actor.setKnowledge("lookat_target", value)
        }
    })
    actor.knowledgeRemoved.connect(function(key) {
        if(key === "movement_target") {
            actor.removeKnowledge("lookat_target")
        }
    })

    calculatePlan(actor);
}

var respawnTime = 50 // in seconds
var bulletCount = 0
var flagTaken = {"green": false, "red": false}
var flagOwner = {"green": null,  "red": null}
function getFlagOwner(color)
{
    return flagOwner[color]
}

function shoot(actor, target)
{
    if(actor.position.distanceTo(target.position) > 7) {
        return;
    }

    var health    = target.getKnowledge("health")
    var newHealth = health - 1

    var shotHeight = 0.5
    var position = actor.position
    position.y += shotHeight
    var bullet = Scene.instantiate("bullet_" + bulletCount, "bullet", position)
    bulletCount++

    var impactPoint = target.position
    impactPoint.y += shotHeight
    bullet.setScale(0.1)
    bullet.setKnowledge("movement_target", impactPoint)
    bullet.setKnowledge("movement_speed", 10)   // in meters per second.
    bullet.knowledgeRemoved.connect(function(key) {
        if(key === "movement_target") {
            Scene.destroyLater(bullet)
        }
    })

    if(newHealth <= 0) {
        var color = target.getKnowledge("team_color")
        var name = target.name
        setTimeout(respawnTime * 1000,
                   function() { 
                        spawnFighter(color, name) 
                   })

        var owner = getFlagOwner(color)
        if(owner !== null &&
           target.name === owner.name) {
            dropFlag(target, color)
        }

        Scene.destroy(target)
    } else {
        target.setKnowledge("health", newHealth)
    }
}

function setAllTeamHasFlags(color, value)
{
    var teamMembers = Scene.getKnowledge("team_" + color)
    for(var member in teamMembers) {
        member.setKnowledge("team_has_flag", value)
    }
}

var global = this
function takeFlag(actor, color)
{
    var otherColor = color === "red" ? "green" : "red"

    actor.setKnowledge("has_flag", true)
    setAllTeamHasFlags(color, true)
    flagTaken[color] = true
    flagOwner[color] = actor

    var flag = global["flag_" + otherColor]

    actor.attach(flag)
    flag.scale = flag.scale.divide(actor.scale)
    flag.position = new Vector3(0,0,0)
}

function dropFlag(actor, color)
{
    var otherColor = color === "red" ? "green" : "red"

    actor.setKnowledge("has_flag", false)
    setAllTeamHasFlags(color, false)
    flagTaken[color] = false
    flagOwner[color] = null

    var flag = global["flag_" + otherColor]
    var flagPosKey = "goal_" + otherColor

    // Reset the flag position
    Scene.attach(flag)
    flag.scale = flag.scale.multiply(actor.scale)
    flag.position = Scene.getKnowledge(flagPosKey)
}

function capture(actor)
{
    var color = actor.getKnowledge("team_color")
    if(flagTaken[color] === true) {
        return;
    }

    takeFlag(actor, color)
}

Scene.setKnowledge("score_green", 0)
Scene.setKnowledge("score_red", 0)
function score(actor)
{
    var color = actor.getKnowledge("team_color")
    if(flagTaken[color] === false &&
       flagOwner[color] !== actor) {
        return;
    }

    var scoreKey = "score_" + color
    var score = Scene.getKnowledge(scoreKey)
    Scene.setKnowledge(scoreKey, score + 1)

    dropFlag(actor, color)
}