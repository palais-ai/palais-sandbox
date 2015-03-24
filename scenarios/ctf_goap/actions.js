require("global.js")

function Action() {
	this.precondition = Planner.prototype.defaultPrecondition
	this.postcondition = Planner.prototype.defaultPostcondition
	this.cost = Planner.prototype.defaultCost
	this.perform = function(actor) {}
}

function MoveToFlagAction(goal) {
	this.precondition = function(state) {
		return true
	}
	this.postcondition = function(state) {
		var ownGoal = state["team_color"] === "red" ? flag_red : flag_green;
		var otherGoal = state["team_color"] === "red" ? flag_green : flag_red;

		if(goal.equals(ownGoal.position) && state["has_flag"] === true) {
			state["has_flag"] = false
			state["made_points"] = true
		} else if(goal.equals(otherGoal.position)) {
			state["has_flag"] = true
		}
		return state;
	}
	this.cost = function(state) {
		return 0
	}
	this.perform = function(actor) {
		print(goal)
	}
}