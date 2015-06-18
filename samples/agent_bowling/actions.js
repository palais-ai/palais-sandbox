function MoveToAction(goal) {
	this.name = "MoveTo_" + goal;
	this.precondition = function(state) {
		return true
	}
	this.postcondition = function(state) {
		state["position"] = goal;
		return state;
	}
	this.cost = function(state) {
		return goal.distanceTo(state["position"])
	}
	this.perform = function(actor, isDone) {
		var called = false;
		actor.setKnowledge("movement_target", goal)
		actor.knowledgeRemoved.connect(function(key) {
			if(key === "movement_target" && !called) {
				called = true
				isDone()
			}
		})
	}
}

function myIndexOf(arr, o) {    
    for (var i = 0; i < arr.length; i++) {
        if (arr[i].name === o.name) {
            return i;
        }
    }
    return -1;
}

function ThrowBarrelAction(barrel, pin) {
	this.name = "ThrowBarrel_" + barrel.name + "_" + pin.name;
	this.precondition = function(state) {
		var barrels = state["barrels"];
		var pins = state["pins"]
		var maxThrowRange = 5;
		return state["position"].equals(barrel.position) && 
			   myIndexOf(barrels, barrel) != -1 && myIndexOf(pins, pin) != -1 &&
			   pin.position.distanceTo(barrel.position) < maxThrowRange
	}
	this.postcondition = function(state) {
		var barrels = state["barrels"];
		barrels.splice(myIndexOf(barrels, barrel), 1);
		var pins = state["pins"];
		pins.splice(myIndexOf(pins, pin), 1);

		state["barrels"] = barrels;
		state["pins"] = pins
		if(pins.length == 0)
		{
			state["all_pins_done"] = true;
		}
		return state;
	}
	this.cost = function(state) {
		return 0
	}
	this.perform = function(actor, isDone) {
		var s = actor.position;
		s.y += 0.2;
		var e = pin.position
		e.y += 0.2
		d.drawLine(s, e, new Color(234/255., 205/255., 24/255.));
		d.drawCircle(e, 0.5, 4, Colors.GREEN);

		Scene.destroyLater(barrel)
		Scene.destroyLater(pin)
		isDone()
	}
}

function ThrowPinAction(pin) {
	this.name = "ThrowPin_" + pin.name;
	this.precondition = function(state) {
		var pins = state["pins"]
		return state["position"].equals(pin.position) && myIndexOf(pins, pin) != -1
	}
	this.postcondition = function(state) {
		var pins = state["pins"];
		pins.splice(myIndexOf(pins, pin), 1);
		state["pins"] = pins
		if(pins.length == 0)
		{
			state["all_pins_done"] = true;
		}
		return state;
	}
	this.cost = function(state) {
		return 0
	}
	this.perform = function(actor, isDone) {
		var s = actor.position;
		s.y += 0.2;
		d.drawCircle(s, 0.5, 4, Colors.GREEN);
		Scene.destroyLater(pin)
		isDone()
	}
}