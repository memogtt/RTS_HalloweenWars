#include "Precompiled.h"
#include "Entity.h"

#include "AIWorld.h"

using namespace AI;

Entity::Entity(AIWorld& world, uint16_t typeId)
	:world(world)
	, id(world.GetNextId(typeId))
{
	world.RegisterEntity(this);
}

Entity::~Entity()
{
	world.UnregisterEntity(this);
}

X::Math::Matrix3 Entity::LocalToWorld() const
{
	return {
		heading.y,-heading.x,0.0f,
		heading.x,heading.y,0.0f,
		position.x, position.y,1.0f

	};
}