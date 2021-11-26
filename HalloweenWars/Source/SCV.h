#pragma once

#include <AI.h>
#include <XEngine.h>

class Player;

class SCV : public AI::Agent
{
public:
	SCV(AI::AIWorld& world);

	void Initialize();

	void Update(float deltaTime);
	void Render();

	bool checkCollision();

	void SetActiveBehaviors(bool seekActive, bool arriveActive, bool wanderActive, bool collisionActive,
		bool separationActive, bool alignmentActive, bool cohesionActive);

	void SetDestinationId(int destId) { mDestinationId = destId; }

	void SetOwner(std::shared_ptr<Player> p) { mOwner = p; }

	std::shared_ptr<Player>& GetOwner() { return mOwner; }
private:
	std::unique_ptr<AI::SteeringModule> mSteeringModule;
	std::array<X::TextureId, 4> mTextureIds;
	int mDestinationId = 0;
	
	std::shared_ptr<Player> mOwner;

	AI::SeekBehavior* mSeekBehavior = nullptr;
	AI::ArriveBehavior* mArriveBehavior = nullptr;
	AI::WanderBehavior* mWanderBehavior = nullptr;
	AI::ObstacleAvoidanceBehavior* mObstacleAvoidanceBehavior = nullptr;

	AI::SeparationBehavior* mSeparationBehavior = nullptr;
	AI::AlignmentBehavior* mAlignmentBehavior = nullptr;
	AI::CohesionBehavior* mCohesionBehavior = nullptr;


};