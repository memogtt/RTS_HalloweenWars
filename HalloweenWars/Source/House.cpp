#include "House.h"

#include "TypeIds.h"
#include "Player.h"
#include "SCV.h"
#include "HouseSpin.h"
#include "HouseAnim0.h"
#include "HouseAnim1.h"
#include "HouseAnim2.h"
#include "HouseAnim3.h"
#include "HouseAnim4.h"
//#include <ImGui/Inc/imgui.h>

House::House(AI::AIWorld& world)
	: AI::Agent(world, TypeIds::House)
{
}

void House::Initialize()
{
	mStateMachine = std::make_unique<AI::StateMachine<House>>(*this);
	//mStateMachine->AddState<HouseSpin>();
	mStateMachine->AddState<HouseAnim0>();
	mStateMachine->AddState<HouseAnim1>();
	mStateMachine->AddState<HouseAnim2>();
	mStateMachine->AddState<HouseAnim3>();
	mStateMachine->AddState<HouseAnim4>();

	mStateMachine->ChangeState(HouseAnim0::GetName());

	mCollisionCircle.center = { position.x,position.y };
	mCollisionCircle.radius = { radius };

}

void House::Update(float deltaTime)
{
	mStateMachine->Update(deltaTime);

	if (mOwner != nullptr)
		mUnits += deltaTime * mRegenRate;
}

void House::Render()
{
	//float angle = atan2(-heading.x, heading.y) + X::Math::kPi;
	//int frame = (int)(angle / X::Math::kTwoPi * mTextureIds.size()) % mTextureIds.size();
	
	//X::DrawSprite(mTextureId, position);
	
	float scale = (mCollisionCircle.radius * 2) / static_cast<float>(X::GetSpriteWidth(mTextureId));	
	X::DrawSprite(mTextureId, position, X::Pivot::Center, X::Flip::None, scale);
	//X::DrawSprite(mTextureId, recto, position);
	//X::GetSpriteWidth
	//X::DrawScreenCircle(mCollisionCircle, GetPlayerColor());

	//debug
	//X::DrawScreenText(std::to_string(mUnits).c_str(), position.x, position.y - 94.0f, 20.0f, X::Colors::White);
	//X::DrawScreenText(std::to_string(GetPlayerOwner()).c_str(), position.x, position.y + 94.0f, 20.0f, X::Colors::Teal);
	//X::DrawScreenText(std::to_string(mPlayerOwner).c_str(), position.x, position.y + 114.0f, 20.0f, X::Colors::Yellow);
	//X::DrawScreenText(std::to_string(id).c_str(), position.x, position.y + 114.0f, 20.0f, X::Colors::White);

	//X::DrawScreenText(std::to_string(cont).c_str(), position.x - 20.0f, position.y, 20.0f, X::Colors::White);

	char currentUnits[8];
	float posOffset = 6.0f;

	if (mUnits > 100)
		posOffset = 16.0f;
	else if(mUnits > 10)
		posOffset = 11.0f;

	sprintf_s(currentUnits, "%.0f", floor(mUnits));
	//X::DrawScreenText(currentUnits, position.x - posOffset, position.y - 10.0f, 20.0f, GetPlayerColor());
	X::DrawScreenText(currentUnits, position.x - posOffset, position.y - 10.0f, 20.0f, X::Colors::White);

	if (mouseOver({ (float)X::GetMouseScreenX(), (float)X::GetMouseScreenY() })) {
		DrawOverInfo();
	}
	//X::DrawScreenLine(position, position + heading * 100.0f, X::Colors::Red);
}

const int House::GetUnitsByPercentage(const int percentage)
{
	int returnUnits{ 0 };
	if (mUnits > 1)
	{
		returnUnits = static_cast<int>(floor(mUnits * percentage / 100));

		if (returnUnits == 0)
			returnUnits = 1;
	}

	return returnUnits;
}

int House::GetPlayerOwner()
{
	return  (mOwner != nullptr) ? mOwner->GetId() : 0;
}

X::Color House::GetPlayerColor()
{
	return  (mOwner != nullptr) ? mOwner->GetColor() : X::Colors::White;
}

void House::ProcessAttack(float unit, std::shared_ptr<Player> p)
{
	cont++;
	if (mOwner != nullptr && p->GetId() == mOwner->GetId())
	{
		mUnits += unit;		
	}
	else
	{
		mUnits -= unit;
		if (mUnits < 0) {
			if (mOwner != nullptr){
				mOwner->mHouseSelected = nullptr;
				
			}			
			mPlayerOwner = p->GetId();
			mOwner = p;
			mUnits = 1.0f;
		}
	}
}

bool House::mouseOver(X::Math::Vector2 position)
{
	return X::Math::PointInCircle(position, mCollisionCircle);
}

void House::sendMonsters(AI::AIWorld& world, std::shared_ptr<House>& enemy, std::vector<std::unique_ptr<SCV>>& scvs)
{
	int numMonsters = GetUnitsByPercentage(mOwner->GetPercentage());
	for (size_t i = 0; i < numMonsters; ++i)
	{
		X::Math::Vector2 randomInsideCircle{ X::Math::Vector2::Zero() };
		//float houseRadius{ X::RandomFloat(20.0f, 64.0f) };

		randomInsideCircle.x = X::RandomFloat(position.x - radius, position.x + radius);
		randomInsideCircle.y = X::RandomFloat(position.y - radius, position.y + radius);

		auto& scv = scvs.emplace_back(std::make_unique<SCV>(world));
		scv->Initialize();
		scv->position = randomInsideCircle;
		scv->destination = enemy->position;
		scv->SetDestinationId(enemy->id);
		scv->SetActiveBehaviors(true, false, false, false, true, false, false);
		scv->SetOwner(mOwner);
		scv->playerMonster = mOwner->GetMonsterNum();
	}
	IncreaseUnits(numMonsters * -1.0f);
}

void House::sendMonsters(AI::AIWorld& world, std::shared_ptr<House>& enemy, std::shared_ptr<Player>& player, std::vector<std::unique_ptr<SCV>>& scvs, int percentage)
{
	int numMonsters = GetUnitsByPercentage(percentage);
	for (size_t i = 0; i < numMonsters; ++i)
	{
		X::Math::Vector2 randomInsideCircle{ X::Math::Vector2::Zero() };
		//float houseRadius{ X::RandomFloat(20.0f, 64.0f) };

		randomInsideCircle.x = X::RandomFloat(position.x - radius, position.x + radius);
		randomInsideCircle.y = X::RandomFloat(position.y - radius, position.y + radius);

		auto& scv = scvs.emplace_back(std::make_unique<SCV>(world));
		scv->Initialize();
		scv->position = randomInsideCircle;
		scv->destination = enemy->position;
		scv->SetDestinationId(enemy->id);
		scv->SetActiveBehaviors(true, false, false, false, true, false, false);
		scv->SetOwner(player);
		scv->playerMonster = mOwner->GetMonsterNum();
	}
	IncreaseUnits(numMonsters * -1.0f);
}

void House::DrawOverInfo()
{
	char regenRate[16];
	sprintf_s(regenRate, "Planet %.0f%%", mRegenRate * 100.0f);
	X::DrawScreenText(regenRate, 500.0f, 5.0f, 20.0f, GetPlayerColor());
}
