#include "Precompiled.h"
#include "WanderBehavior.h"

#include "Agent.h"

using namespace AI;

X::Math::Vector2 WanderBehavior::Calculate(Agent& agent)
{



	/*
	public:
		X::MATH::VECTOR2 WANDERTARGETLOCAL = X:MATH::VECTOR2::YAXIS;

		FLOAT
		WANDER RADIUS 25
		WANDERDISTANCE 100
		WANDERRJITTER = 1

		CALCULATE

		AUTO PROJECTEDTARGET = WANDERTARGET LOCAL + VEC2(0, WANDERDISTANCE);
		AUTO WORLDTARGET = X::MATH::TRANSFORMCOORD(PROJECTEDTARGET, AGENT.LOCALTOWORLD()); //AGENTLOCALTOWORLD
		SEEK TO WORLDTARGET;

		X::DRAWSCREENCIRCLE()




	*/

	//auto random = X::RandomVector2(0.0f,mWanderJitter);
	//float yWander = X::RandomFloat(-mWanderJitter, mWanderJitter);
	auto random = X::RandomVector2(-mWanderJitter,mWanderJitter);

	//X::Math::Vector2 random = { 0.0f,X::RandomFloat(-mWanderJitter, mWanderJitter) };

	//we get the local wander plus a random location
	//X::Math::Vector2 newTarget = mLocalWanderTarget + random;
	X::Math::Vector2 newTarget = X::Math::Vector2::YAxis() + random;
	//we pull that new target to our circle normalizing and multiplying by wanderRadius
	mLocalWanderTarget = X::Math::Normalize(newTarget) * mWanderRadius;

	//paint all this local circle and wander random location and draw in our local circle
	//X::DrawScreenCircle(agent.position, mWanderRadius, X::Colors::Blue);
	//X::DrawScreenDiamond(X::Math::TransformCoord(random, agent.LocalToWorld()), 5.0f, X::Colors::Green);
	//X::DrawScreenDiamond(X::Math::TransformCoord(mLocalWanderTarget, agent.LocalToWorld()), 1.0f, X::Colors::Yellow);

	//we project our new projected target
	X::Math::Vector2 projectedTarget = mLocalWanderTarget + X::Math::Vector2(0, mWanderDistance);

	//we transform our new projectedTarget to world position and set it as destination
	X::Math::Vector2 worldWanderTarget = X::Math::TransformCoord(projectedTarget, agent.LocalToWorld());
	agent.destination = worldWanderTarget;

	auto wanderCircle = X::Math::TransformCoord(X::Math::Vector2(0, mWanderDistance), agent.LocalToWorld());
	X::DrawScreenCircle(wanderCircle, mWanderRadius,X::Colors::Blue);
	X::DrawScreenDiamond(worldWanderTarget, 1.0f, X::Colors::Yellow);

	//seek logic
	auto toTarget = agent.destination - agent.position;
	auto desiredVelocity = X::Math::Normalize(toTarget) * agent.maxSpeed;
	return desiredVelocity - agent.velocity;
}