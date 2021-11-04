#include "Precompiled.h"
#include "SeekBehavior.h"

#include "Agent.h"

using namespace AI;

X::Math::Vector2 SeekBehavior::Calculate(Agent& agent)
{
	auto toTarget = agent.destination - agent.position;
	auto desiredVelocity = X::Math::Normalize(toTarget) * agent.maxSpeed;
	auto seekForce = desiredVelocity - agent.velocity;

	if (ShowDebug())
	{
		X::DrawScreenLine(agent.position, agent.position + desiredVelocity, X::Colors::Yellow);
		X::DrawScreenLine(agent.position, agent.position + agent.velocity, X::Colors::Green);
		X::DrawScreenLine(agent.position + agent.velocity, agent.position + agent.velocity + seekForce, X::Colors::Red);
	}

	return seekForce;
}