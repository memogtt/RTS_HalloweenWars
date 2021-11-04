#pragma once

#include "Entity.h"

namespace AI
{
	class Agent;

	using Agents = std::vector<Agent*>;

	class Agent : public Entity
	{
	public:
		Agent(AIWorld& world, uint16_t typeId)
			:Entity(world, typeId)
		{}

		Agents neighbors;
		X::Math::Vector2 velocity = X::Math::Vector2::Zero();
		X::Math::Vector2 destination = X::Math::Vector2::Zero();
		float maxSpeed = 1.0f;
		float mass = 1.0f;

	};
}