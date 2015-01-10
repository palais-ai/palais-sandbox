require("global.js")

function Action() {
	this.precondition = Planner.prototype.defaultPrecondition
	this.postcondition = Planner.prototype.defaultPostcondition
	this.cost = Planner.prototype.defaultCost
	this.perform = function(actor) {}
}

function MoveToAction(goal) {
	this.precondition = function(state) {

	}
	this.postcondition = function(state) {

	}
	this.cost = function(state) {

	}
	this.perform = function(state) {

	}
}