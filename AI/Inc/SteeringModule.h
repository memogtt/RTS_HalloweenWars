#pragma once

#include "SteeringBehavior.h"

namespace AI
{
	class SteeringModule
	{
	public:
		SteeringModule(Agent& agent);

		template <class BehaviorType>
		BehaviorType* AddBehavior();

		X::Math::Vector2 Calculate();

	private:
		//using BehaviorMap = std::unordered_map<std::string, std::unique_ptr<SteeringBehavior>>;
		using Behaviors = std::vector<std::unique_ptr<SteeringBehavior>>;

		Agent& mAgent;
		Behaviors mBehaviors;
	};

	template <class BehaviorType>
	BehaviorType* SteeringModule::AddBehavior()
	{
		static_assert(std::is_base_of_v<SteeringBehavior, BehaviorType>,
			"BehaviorType os not a child of AI::SteeringBehavior!!");

		auto& newBehavior = mBehaviors.emplace_back(std::make_unique <BehaviorType>());
		return static_cast<BehaviorType*>(newBehavior.get());

		//auto [iter, success] = mBehaviors.try_emplace(std::move(name), std::make_unique<BehaviorType>());
		//return success ? static_cast<BehaviorType*>(iter->second.get()) : nullptr;
	}
}