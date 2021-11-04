#include "Precompiled.h"
#include "ObstacleAvoidanceBehavior.h"

#include "Agent.h"
#include "AIWorld.h"

using namespace AI;

X::Math::Vector2 ObstacleAvoidanceBehavior::Calculate(Agent& agent)
{
	auto& obstacles = agent.world.GetObstacles();

	float avoidanceDistance = 200.0f + (X::Math::Magnitude(agent.velocity) / agent.maxSpeed) * 300.0f;

	
	auto toWorld = agent.LocalToWorld();
	auto toLocal = X::Math::Inverse(toWorld);
	auto worldPoint = X::Math::TransformCoord(agent.position, toWorld);
	//auto localPoint = X::Math::TransformCoord(worldPoint, toLocal);

	auto localPosition = X::Math::TransformCoord(agent.position, toLocal);

	//X::DrawScreenText(std::to_string(agent.position.x).c_str(), 300.0f, 300.0f, 20.0f, X::Colors::White);
	//X::DrawScreenText(std::to_string(agent.position.y).c_str(), 300.0f, 320.0f, 20.0f, X::Colors::White);
	//X::DrawScreenText(std::to_string(localPosition.x).c_str(), 350.0f, 350.0f, 20.0f, X::Colors::White);
	//X::DrawScreenText(std::to_string(localPosition.y).c_str(), 350.0f, 370.0f, 20.0f, X::Colors::White);

	std::vector<X::Math::Circle> localObstacles;
	for (auto& worldObs : obstacles)
	{
		//skip obstacles that are too far
		if (X::Math::Distance(agent.position, worldObs.center) > avoidanceDistance)
			continue;
		//transform the circle to local
		X::Math::Vector2 localObstacleCenter = X::Math::TransformCoord(worldObs.center, toLocal);

		//skip obstacles that are behind
		if (localObstacleCenter.y < 0)
			continue;

	
		//we add the agent raidius
		localObstacles.push_back(X::Math::Circle(localObstacleCenter, worldObs.radius + agent.radius));
	}

	float closestY = 1000.0f;
	bool steerleft = true;
	bool obstaclefound = false;
	float strenght = 0.0f;
	X::Math::Circle closestCircle;
	for (auto& localObs : localObstacles)
	{
		//skip obstacles that dont reach the center line
		if (X::Math::Abs(localObs.center.x) > localObs.radius)
			continue;

		//apply equation of circle with x=0
		//keep smallest positive closestY

		X::DrawScreenDiamond(X::Math::TransformCoord({ 0, -X::Math::Sqrt(X::Math::Sqr(localObs.radius) - X::Math::Sqr(localObs.center.x)) + localObs.center.y }, toWorld), 10.0f, X::Colors::Purple);
		X::DrawScreenDiamond(X::Math::TransformCoord({ 0, X::Math::Sqrt(X::Math::Sqr(localObs.radius) - X::Math::Sqr(localObs.center.x)) + localObs.center.y }, toWorld), 10.0f, X::Colors::Green);

		float current_y = -X::Math::Sqrt(X::Math::Sqr(localObs.radius) - X::Math::Sqr(localObs.center.x)) + localObs.center.y;
		if (current_y < closestY)
		{
			closestY = current_y;
			if (localObs.center.x > 0) {
				steerleft = false;	
			}
			strenght = localObs.radius - localObs.center.x;
			obstaclefound = true;
			closestCircle = localObs;
		}
		//X::DrawScreenDiamond(X::Math::TransformCoord({ closestY,0 },toWorld), 10.0f, X::Colors::Red);
		
		/*closestY = squareroot
		if (-squareroot + localObs.center.x < closestY)
			closestY = -squareroot - localObs.center.x;*/

	}

	if (obstaclefound) {
		X::DrawScreenDiamond(X::Math::TransformCoord({ 0, closestY }, toWorld), 10.0f, X::Colors::Red);

		//X::Math::Vector2 collisionPoint= { 0, closestY };
		//X::Math::Vector2 agentLocalPos= { 0, 0};
		X::Math::Vector2 toTarget = { closestY,0 };
		auto desiredVelocity = X::Math::Normalize(toTarget) * agent.maxSpeed;
		auto seekForce = desiredVelocity - agent.velocity;

		X::Math::Vector2 destination;
		if (steerleft) {
			destination = { closestY,0 };
		}
		else {
			destination = { -closestY,0 };
		}

		auto lenghtDestination = X::Math::Magnitude(destination);
		auto lateralForce = destination / lenghtDestination * agent.maxSpeed;

		/*X::Math::Vector2 lateralForce{ closestCircle.radius - closestCircle.center.x, 0.0f };
		if (closestCircle.center.x < 0)
			lateralForce.x = -lateralForce.x;*/

		X::DrawScreenLine(agent.position, X::Math::TransformCoord(agent.position + lateralForce,toWorld), X::Colors::Red);
		X::DrawScreenText(std::to_string(closestCircle.radius - closestCircle.center.x).c_str(), 300.0f, 300.0f, 20.0f, X::Colors::White);

	X::Math::Vector2  collisionPoint = { 0, closestY };
	auto lenghtSqr = X::Math::MagnitudeSqr(-collisionPoint);
	auto lenght = X::Math::Magnitude(-collisionPoint);
	auto brakingForce = -collisionPoint / lenghtSqr * agent.maxSpeed;

	X::Math::Vector2 localAvoidanceForce = lateralForce + brakingForce;
	return X::Math::TransformNormal(localAvoidanceForce, toWorld);
	//return X::Math::Vector2();
	}
	else
	{
		return X::Math::Vector2();
	}
}