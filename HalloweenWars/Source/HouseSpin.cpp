#include "HouseSpin.h"

#include "House.h"
//#include "KnightDead.h"
//#include "KnightAttack.h"
//#include "KnightRun.h"
//#include "KnightJump.h"

void HouseSpin::Enter(House& agent)
{
	timer = 0.0f;
	currentTexture = -1;
	mAnimTime = 0.0f;
	
	planet = (agent.GetNetworkHouseId() % 11)+1;
	//planet = (agent % 11)+1;

	if (planet < 9)
	{
		for (size_t i = 0; i < mTextureIds.size(); ++i)
		{
			char name[256];
			//sprintf_s(name, "Moon/idle%02zu.png", i + 1);
			sprintf_s(name, "Planets/%01zu/%01zu.png", planet, i + 1);
			mTextureIds[i] = X::LoadTexture(name);
		}
	}
	else
	{
		for (size_t i = 0; i < mTextureIds2.size(); ++i)
		{
			char name[256];
			//sprintf_s(name, "Moon/idle%02zu.png", i + 1);
			sprintf_s(name, "Planets/%01zu/%01zu.png", planet, i + 1);
			mTextureIds2[i] = X::LoadTexture(name);
		}
	}
}

void HouseSpin::Update(House& agent, float deltaTime)
{
	if (mAnimTime < X::GetTime()) {
		currentTexture++;

		if (planet < 9)
		{
			currentTexture %= mTextureIds.size();
			agent.mTextureId = mTextureIds[currentTexture];
		}
		else
		{
			currentTexture %= mTextureIds2.size();
			agent.mTextureId = mTextureIds2[currentTexture];
		}

		mAnimTime = X::GetTime() + 15 * deltaTime;
	}

	//if (agent.mHealth <= 0)
	//{
	//	agent.mStateMachine->ChangeState(KnightDead::GetName());
	//}

	//if (X::IsKeyDown(X::Keys::A))
	//{
	//	agent.mStateMachine->ChangeState(KnightAttack::GetName());
	//}

	//if (X::IsKeyDown(X::Keys::RIGHT))
	//{
	//	agent.mIsFacingLeft = false;
	//	agent.mStateMachine->ChangeState(KnightRun::GetName());
	//}

	//if (X::IsKeyDown(X::Keys::LEFT))
	//{
	//	agent.mIsFacingLeft = true;
	//	agent.mStateMachine->ChangeState(KnightRun::GetName());
	//}

	//if (X::IsKeyDown(X::Keys::SPACE))
	//{
	//	agent.mStateMachine->ChangeState(KnightJump::GetName());
	//}
}

