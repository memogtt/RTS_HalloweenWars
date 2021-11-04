#pragma once

#include "SteeringBehavior.h"

namespace AI
{
	class SeekBehavior : public SteeringBehavior
	{
	public:
		X::Math::Vector2 Calculate(Agent& agent);
	};
}