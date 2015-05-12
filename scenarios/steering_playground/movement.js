function seek(agent, targetPosition, maxSpeed /* in m / s */) {
	return targetPosition.subtract(agent.position).normalize().multiply(maxSpeed);
}

function flee(agent, targetPosition, maxSpeed /* in m / s */) {
	return targetPosition.subtract(agent.position).normalize().multiply(-maxSpeed);
}

function movementBehavior(agent, targetPosition, maxSpeed /* in m / s */) {
	return seek(agent, targetPosition, maxSpeed);
	//return flee(agent, targetPosition, maxSpeed);
	//return new Vector3(0,0,0);
}