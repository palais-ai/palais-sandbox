function shoot(actor, target)
{
    var health    = target.getKnowledge("health");
    var newHealth = health - 1;

    print(actor.name + " shot " + target.name + ". Bang. [" + health + " -> " + newHealth + "]");

    if(newHealth <= 0)
    {
        Scene.destroy(target);
    }
    else
    {
        target.setKnowledge("health", newHealth);
    }
}

function setAllTeamHasFlags(color, value)
{
    var teamMembers = Scene.getKnowledge("team_" + color);

    for(var member in teamMembers)
    {
        member.setKnowledge("team_has_flag", value);
    }
}

var global = this;
var flagTaken = {"green": false, "red": false};
var flagOwner = {"green": null,  "red": null};
function getFlagOwner(color)
{
    return flagOwner[color];
}

function capture(actor)
{
    var color = actor.getKnowledge("team_color");
    if(flagTaken[color] === true)
    {
        return;
    }

    var otherColor = color === "red" ? "green" : "red";

    actor.setKnowledge("has_flag", true);
    setAllTeamHasFlags(color, true);
    flagTaken[color] = true;
    flagOwner[color] = actor;

    var flag = global["flag_" + otherColor];

    actor.attach(flag);
    flag.scale = flag.scale.divide(actor.scale);
    flag.position = new Vector3(0,0,0);
}

Scene.setKnowledge("score_green", 0);
Scene.setKnowledge("score_red", 0);
function score(actor)
{
    var color = actor.getKnowledge("team_color");
    if(flagTaken[color] === false &&
       flagOwner[color] !== actor)
    {
        return;
    }

    var otherColor = color === "red" ? "green" : "red";

    var scoreKey = "score_" + color;
    var score = Scene.getKnowledge(scoreKey);
    Scene.setKnowledge(scoreKey, score + 1);

    actor.setKnowledge("has_flag", false);
    setAllTeamHasFlags(color, false);
    flagTaken[color] = false;
    flagOwner[color] = null;

    var flagPosKey = "goal_" + otherColor;
    var flag = global["flag_" + otherColor];

    // Reset the flag position
    Scene.attach(flag);
    flag.scale = flag.scale.multiply(actor.scale);
    flag.position = Scene.getKnowledge(flagPosKey);

    Scene.destroy(actor);
}