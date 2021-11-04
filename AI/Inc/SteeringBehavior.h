#pragma once

#include "Common.h"

namespace AI
{
	class Agent;

	class SteeringBehavior
	{
	public:
		SteeringBehavior() = default;
		virtual ~SteeringBehavior() = default;

		virtual X::Math::Vector2 Calculate(Agent& agent) = 0;

		void SetWeight(bool weight) { mWeight = weight; }
		bool getWeight() const { return mWeight; }
			
		void SetActive(bool active) { mActive = active; }
		bool IsActive() const { return mActive; }

		void EnableDebug(bool debug) { mShowDebug = debug; }
		bool ShowDebug() const { return mShowDebug; }

	private:
		float mWeight = 1.0f;
		bool mActive = false;
		bool mShowDebug = false;
	};
}
