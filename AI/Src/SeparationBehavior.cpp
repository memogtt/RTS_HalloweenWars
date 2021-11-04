#include "Precompiled.h"
#include "SeparationBehavior.h"

#include "Agent.h"

using namespace AI;

X::Math::Vector2 SeparationBehavior::Calculate(Agent& agent)
{
	X::Math::Vector2 separation;

	for (auto& neighbor : agent.neighbors)
	{
		//dont compute force for self
	/*	if (neighbor == &agent)
			continue;*/

		auto neighborToAgent = agent.position - neighbor->position;
		auto lenghtSqr = X::Math::MagnitudeSqr(neighborToAgent);
		if (lenghtSqr <= 0.0f)
			continue;

		X::DrawScreenLine(agent.position, agent.position + ((neighborToAgent / X::Math::MagnitudeSqr(neighborToAgent)) * agent.maxSpeed*getWeight()), X::Colors::Yellow);
		//X::DrawScreenLine(agent.position, agent.position + (lenghtSqr), X::Colors::Green);
		separation += (neighborToAgent / lenghtSqr) * agent.maxSpeed;
	}

	if (ShowDebug())
	{
	}

	return separation;
}