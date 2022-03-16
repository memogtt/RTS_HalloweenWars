#pragma once

#include <AI.h>
#include <XEngine.h>

class Player;
class SCV;

class House : public AI::Agent
{
public:
	House(AI::AIWorld& world);

	void Initialize();

	void Update(float deltaTime);
	void Render();

	//void SetActiveBehaviors(bool seekActive, bool arriveActive, bool wanderActive, bool collisionActive,
	//	bool separationActive, bool alignmentActive, bool cohesionActive);

	const float GetRegenRate() const { return mRegenRate; }
	void SetRegenRate(float rate) { mRegenRate = rate; }

	const float GetUnits() const { return mUnits; }
	void SetUnits(const float& unit) { mUnits = unit; }

	void IncreaseUnits(float unit) { mUnits += unit; }
	const int GetUnitsByPercentage(const int percentage);

	const int GetPlayerNetworkId() const { return mPlayerOwner; }
	int GetPlayerOwner();
	X::Color GetPlayerColor();

	void SetPlayerOwner(const int numPlayer) { mPlayerOwner = numPlayer; }

	void ProcessAttack(float unit, std::shared_ptr<Player> p);

	bool mouseOver(X::Math::Vector2 position);
	void sendMonsters(AI::AIWorld& world, std::shared_ptr<House>& enemy, std::vector<std::unique_ptr<SCV>>& scvs);
	void sendMonsters(AI::AIWorld& world, std::shared_ptr<House>& enemy, std::shared_ptr<Player>& player, std::vector<std::unique_ptr<SCV>>& scvs,int percentage);


	void SetOwner(std::shared_ptr<Player> p) { mOwner = p; }
	std::shared_ptr<Player>& GetOwner() { return mOwner; }

	
	X::Math::Circle getCircle() { return mCollisionCircle; }

	void SetNetworkHouseId(const int id) { mNetworkHouseId = id; }
	const int GetNetworkHouseId() const { return mNetworkHouseId; }	

	std::unique_ptr<AI::StateMachine<House>> mStateMachine;
	X::TextureId mTextureId = 0;

private:
	//std::unique_ptr<AI::SteeringModule> mSteeringModule;
	//std::array<X::TextureId, 16> mTextureIds;
	X::Math::Circle mCollisionCircle{ position.x,position.y,radius };
	float mRegenRate = 0.0f;
	float mUnits = 0.0f;
	int mPlayerOwner = 0;
	int cont = 0;
	std::shared_ptr<Player> mOwner;
	int mNetworkHouseId = -1;

	//AI::SeekBehavior* mSeekBehavior = nullptr;
	//AI::ArriveBehavior* mArriveBehavior = nullptr;
	//AI::WanderBehavior* mWanderBehavior = nullptr;
	//AI::ObstacleAvoidanceBehavior* mObstacleAvoidanceBehavior = nullptr;

	//AI::SeparationBehavior* mSeparationBehavior = nullptr;
	//AI::AlignmentBehavior* mAlignmentBehavior = nullptr;
	//AI::CohesionBehavior* mCohesionBehavior = nullptr;


	void DrawOverInfo();

};

