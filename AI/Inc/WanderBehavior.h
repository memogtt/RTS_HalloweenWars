#pragma once

#include "SteeringBehavior.h"

namespace AI
{
	class WanderBehavior : public SteeringBehavior
	{
	private:
		X::Math::Vector2 mLocalWanderTarget;
		float mWanderRadius = 50.0f;
		float mWanderDistance = 100.0f;
		float mWanderJitter = 50.0f;
	public:
		X::Math::Vector2 Calculate(Agent& agent);
	};
}