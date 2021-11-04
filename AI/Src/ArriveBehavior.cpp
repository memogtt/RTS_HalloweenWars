#include "Precompiled.h"
#include "ArriveBehavior.h"

#include "Agent.h"

using namespace AI;

X::Math::Vector2 ArriveBehavior::Calculate(Agent& agent)
{
	X::Math::Vector2 force = X::Math::Vector2::Zero();
	float dist = X::Math::Distance(agent.destination, agent.position);
	if (dist > 0)
	{
		float speed = dist / 2.5f;
		speed = X::Math::Min(speed, agent.maxSpeed);

		auto toTarget = agent.destination - agent.position;
		auto desiredVelocity = toTarget * speed / dist;
		force = desiredVelocity - agent.velocity;
	}

	return force;
}