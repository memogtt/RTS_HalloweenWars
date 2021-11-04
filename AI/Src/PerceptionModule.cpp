#include "Precompiled.h"
#include "PerceptionModule.h"

using namespace AI;

AI::PerceptionModule::PerceptionModule(Agent& agent, ImportanceCalculator calculator)
	: mAgent(agent)
	, ComputeImportance(std::move(calculator))
{
}

void AI::PerceptionModule::Update(float deltaTime)
{
	//Update sensors and refresh/add memory records
	for (auto& sensor : mSensors)
	{
		sensor->Update(mAgent, mMemory, deltaTime);
	}

	// remove any records that are too old
	//TODO: FIX CODE CLASS 10 8:05PM
	//mMemory.erase(std::remove_if(mMemory.begin)

	for (auto iter = mMemory.begin(); iter != mMemory.end();)
	{
		if (iter->lastRecordedTime + mMemorySpan < X::GetTime())
			iter = mMemory.erase(iter);
		else
			++iter;
	}

	//Calculate importance on remaining records

	for (auto& record : mMemory)
	{
		record.importance = ComputeImportance(mAgent, record);
	}

	//sort records by importance
	mMemory.sort([](const auto& a, const auto& b)
	{
		return a.importance, b.importance;
	});
}