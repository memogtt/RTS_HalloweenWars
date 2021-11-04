#pragma once

#include "Entity.h"
#include "Agent.h"
#include "PartitionGrid.h"

namespace AI
{
	class AIWorld
	{
	public:
		using Obstacles = std::vector <X::Math::Circle>;
		using Walls = std::vector<X::Math::LineSegment>;

		void Initialize(const X::Math::Vector2& worldSize);
		void Update();

		void RegisterEntity(Entity* entity);
		void UnregisterEntity(Entity* entity);

		void AddObstacle(const X::Math::Circle& obstacle);
		void AddWall(const X::Math::LineSegment& wall);

		Agents Getneighborhood(const X::Math::Circle& range, uint16_t typeId);

		int GetNextId(uint16_t typeId);
		uint16_t GetType(int id) const;

		const Obstacles& GetObstacles() const { return mObstacles; }
		const Walls& GetWalls() const { return mWalls; }



	private:
		Entities mEntities;
		Obstacles mObstacles;
		Walls mWalls;
		PartitionGrid<Entity> mGrid;
		int mNextId = 0;
	};
}