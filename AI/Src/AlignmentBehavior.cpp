#include "Precompiled.h"
#include "AlignmentBehavior.h"

#include "Agent.h"

using namespace AI;

X::Math::Vector2 AlignmentBehavior::Calculate(Agent& agent)
{
	X::Math::Vector2 alignment;
	
	if (agent.neighbors.size() <= 1)
		return alignment;

	X::Math::Vector2 averageAlignment;//added extra variable to make it more readable

	for (auto& neighbor : agent.neighbors)
	{
		averageAlignment += neighbor->heading;
		//dont compute force for self
		//if (neighbor == &agent)
		//	continue;

		//auto neighborToAgent = agent.position - neighbor->position;
		//auto lenghtSqr = X::Math::MagnitudeSqr(neighborToAgent);
		//if (lenghtSqr <= 0.0f)
		//	continue;

		//X::DrawScreenLine(agent.position, agent.position + ((neighborToAgent / X::Math::MagnitudeSqr(neighborToAgent)) * agent.maxSpeed * getWeight()), X::Colors::Yellow);
		//X::DrawScreenLine(agent.position, agent.position + (lenghtSqr), X::Colors::Green);
		
	}
	averageAlignment = averageAlignment / agent.neighbors.size();

	X::DrawScreenLine(agent.position, agent.position + averageAlignment *100, X::Colors::White);

	alignment = averageAlignment - agent.heading;
	if (ShowDebug())
	{
	}

	return alignment*100;
}