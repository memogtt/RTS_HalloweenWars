#include "HouseAnim0.h"

#include "House.h"
#include "HouseAnim1.h"
#include "HouseAnim2.h"


void HouseAnim0::Enter(House& agent)
{
	timer = 0.0f;
	currentTexture = -1;
	mAnimTime = 0.0f;

	for (size_t i = 0; i < mTextureIds.size(); ++i)
	{
		char name[256];
		//sprintf_s(name, "Moon/idle%02zu.png", i + 1);
		sprintf_s(name, "Planets/1/%01zu.png", i + 1);
		mTextureIds[i] = X::LoadTexture(name);
	}

}

void HouseAnim0::Update(House& agent, float deltaTime)
{

	if (agent.GetPlayerNetworkId() != 0)
	{
		switch (agent.GetPlayerNetworkId()) {
		case 1:
			agent.mStateMachine->ChangeState(HouseAnim1::GetName());
			break;

		case 2:
			agent.mStateMachine->ChangeState(HouseAnim2::GetName());
			break;

		}

		
	}

	if (mAnimTime < X::GetTime()) {
		currentTexture++;
		currentTexture %= mTextureIds.size();
		agent.mTextureId = mTextureIds[currentTexture];
		mAnimTime = X::GetTime() + 20 * deltaTime;
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

