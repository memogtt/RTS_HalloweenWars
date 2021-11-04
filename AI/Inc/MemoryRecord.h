#pragma once

#include <XEngine.h>

namespace AI
{
	using Property = std::variant<int, float, X::Math::Vector2>;

	struct MemoryRecord
	{
		std::unordered_map<std::string, Property> properties;
		int entityId = 0;
		float lastRecordedTime = 0.0f;
		float importance = 0.0f;
	};

	using MemoryRecords = std::list<MemoryRecord>;
}