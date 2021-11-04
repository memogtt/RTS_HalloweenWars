#include "Precompiled.h"
#include "CohesionBehavior.h"

#include "Agent.h"

using namespace AI;

X::Math::Vector2 CohesionBehavior::Calculate(Agent& agent)
{
	X::Math::Vector2 cohesion{};

	if (agent.neighbors.size() <= 1)
		return cohesion;

	X::Math::Vector2 averagePosition;//added extra variable to make it more readable

	for (auto& neighbor : agent.neighbors)
	{
		//dont compute force for self
		//if (neighbor == &agent)
		//	continue;

		averagePosition += neighbor->position;


		//X::DrawScreenLine(agent.position, agent.position + ((neighborToAgent / X::Math::MagnitudeSqr(neighborToAgent)) * agent.maxSpeed * getWeight()), X::Colors::Yellow);
		//X::DrawScreenLine(agent.position, agent.position + (lenghtSqr), X::Colors::Green);
		//separation += (neighborToAgent / lenghtSqr) * agent.maxSpeed;
	}

	averagePosition = averagePosition / agent.neighbors.size();
	//averagePosition = averagePosition / (agent.neighbors.size()-1);
	//X::DrawScreenDiamond(X::Math::TransformCoord(averagePosition, toWorld), 10.0f, X::Colors::Red);
	X::DrawScreenDiamond(averagePosition, 10.0f, X::Colors::Red);

	auto toTarget = averagePosition - agent.position;
	auto desiredVelocity = X::Math::Normalize(toTarget) * agent.maxSpeed;
	cohesion = desiredVelocity - agent.velocity;

	//if (ShowDebug())
	//{
	//}

	return cohesion;
}