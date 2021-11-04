#pragma once

namespace AI
{
	class Entity;
	class AIWorld;

	class Entity
	{
	public:
		//Entity(AIWorld& world, int typeId);
		Entity(AIWorld& world, uint16_t typeId);
		virtual ~Entity();

		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;

		Entity(Entity&&) = delete;
		Entity& operator=(Entity&&) = delete;

		X::Math::Matrix3 LocalToWorld() const;


		AIWorld& world;
		X::Math::Vector2 position = X::Math::Vector2::Zero();
		X::Math::Vector2 heading = X::Math::Vector2::YAxis();
		const int id = 0;
		float radius = 1.0f;

	};

	using Entities = std::vector<Entity*>;
}