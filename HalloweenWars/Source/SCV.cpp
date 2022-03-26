#include "SCV.h"
#include "House.h"
#include "Player.h"

#include "TypeIds.h"
#include <ImGui/Inc/imgui.h>

#include "SCVSkeleton.h"
#include "SCVPumpkinhead.h"
#include "SCVGhost.h"
#include "SCVAlien.h"

#include "SCV.h"

SCV::SCV(AI::AIWorld& world)
	: AI::Agent(world, TypeIds::SCV)//, mPlayerOwner(2)
{
}

void SCV::Initialize()
{
	maxSpeed = 200.0f;

	mStateMachine = std::make_unique<AI::StateMachine<SCV>>(*this);
	//mStateMachine->AddState<HouseSpin>();
	mStateMachine->AddState<SCVSkeleton>();
	mStateMachine->AddState<SCVPumpkinhead>();
	mStateMachine->AddState<SCVGhost>();
	mStateMachine->AddState<SCVAlien>();

	mStateMachine->ChangeState(SCVSkeleton::GetName());

	mSteeringModule = std::make_unique<AI::SteeringModule>(*this);
	mSeekBehavior = mSteeringModule->AddBehavior<AI::SeekBehavior>();
	//mSeekBehavior->SetActive(true);

	mArriveBehavior = mSteeringModule->AddBehavior<AI::ArriveBehavior>();
	//mArriveBehavior->SetActive(true);

	mWanderBehavior = mSteeringModule->AddBehavior<AI::WanderBehavior>();
	//mWanderBehavior->SetActive(true);

	mObstacleAvoidanceBehavior = mSteeringModule->AddBehavior<AI::ObstacleAvoidanceBehavior>();
	//mObstacleAvoidanceBehavior->SetActive(true);

	mSeparationBehavior = mSteeringModule->AddBehavior<AI::SeparationBehavior>();
	mSeparationBehavior->SetWeight(0.1f);
	//mSeparationBehavior->SetActive(true);
	mAlignmentBehavior = mSteeringModule->AddBehavior<AI::AlignmentBehavior>();
	mCohesionBehavior = mSteeringModule->AddBehavior<AI::CohesionBehavior>();

	mSeekBehavior->EnableDebug(false);
	mSeparationBehavior->EnableDebug(false);



	//for (size_t i = 0; i < mTextureIds.size(); ++i)
	//{
	//	char name[256];
	//	sprintf_s(name, "cat_%02zu.png", i + 1);
	//	mTextureIds[i] = X::LoadTexture(name);
	//}
}

bool SCV::checkCollision()
{

	neighbors = world.Getneighborhood({ position,100.0f }, TypeIds::House);

	for (auto& neighbor : neighbors) {
		if (neighbor->id == mDestinationId && X::Math::PointInCircle(position, { neighbor->position,neighbor->radius })) {

			static_cast<House*>(neighbor)->ProcessAttack(1.0f, mOwner);
			return true;
		}
	}
	return false;
}

void SCV::Update(float deltaTime)
{
	neighbors = world.Getneighborhood({ position,100.0f }, TypeIds::SCV);

	auto force = mSteeringModule->Calculate();
	auto acceleration = force / mass;
	velocity += acceleration * deltaTime;

	// truncate to max speed here
	if (velocity.x > 0)
		velocity.x = X::Math::Min(velocity.x, maxSpeed);
	else
		velocity.x = X::Math::Max(velocity.x, -maxSpeed);

	if (velocity.y > 0)
		velocity.y = X::Math::Min(velocity.y, maxSpeed);
	else
		velocity.y = X::Math::Max(velocity.y, -maxSpeed);


	//X::DrawScreenText(std::to_string(velocity.x).c_str(), 350.0f, 300.0f, 20.0f, X::Colors::White);
	//X::DrawScreenText(std::to_string(velocity.y).c_str(), 350.0f, 370.0f, 20.0f, X::Colors::White);

	position += velocity * deltaTime;

	if (X::Math::MagnitudeSqr(velocity) > 1.0f)
		heading = X::Math::Normalize(velocity);

	// wrapping logic for all sides here
	auto screenWidth = X::GetScreenWidth();
	auto screenHeight = X::GetScreenHeight();
	if (position.x >= screenWidth)
		position.x -= screenWidth;
	if (position.y >= screenHeight)
		position.y -= screenHeight;
	if (position.x < 0.0f)
		position.x += screenWidth;
	if (position.y < 0.0f)
		position.y += screenHeight;

	mStateMachine->Update(deltaTime);

}

void SCV::Render()
{
	float angle = atan2(-heading.x, heading.y) + X::Math::kPi;
	//int frame = (int)(angle / X::Math::kTwoPi * mTextureIds.size()) % mTextureIds.size();
	//X::DrawSprite(mTextureIds[frame], position);

	//X::DrawScreenLine(position, position + heading * 100.0f, X::Colors::Red);
	//X::DrawScreenLine(position, position + heading * 100.0f, mOwner->GetColor());
	//X::DrawScreenCircle({ position.x,position.y,20.0f }, mOwner->GetColor());

	X::DrawSprite(mTextureId, position, X::Pivot::Center, X::Flip::None);
}

void SCV::SetActiveBehaviors(bool seekActive, bool arriveActive, bool wanderActive, bool collisionActive,
	bool separationActive, bool alignmentActive, bool cohesionActive)
{
	mSeekBehavior->SetActive(seekActive);
	mArriveBehavior->SetActive(arriveActive);
	mWanderBehavior->SetActive(wanderActive);
	mObstacleAvoidanceBehavior->SetActive(collisionActive);
	mSeparationBehavior->SetActive(separationActive);
	mAlignmentBehavior->SetActive(alignmentActive);
	mCohesionBehavior->SetActive(cohesionActive);
}
