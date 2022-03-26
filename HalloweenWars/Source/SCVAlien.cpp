#include "SCVAlien.h"

#include "SCV.h"
#include "SCVSkeleton.h"
#include "SCVPumpkinhead.h"
#include "SCVGhost.h"


void SCVAlien::Enter(SCV& agent)
{
	timer = 0.0f;
	currentTexture = -1;
	mAnimTime = 0.0f;

	for (size_t i = 0; i < mTextureIds.size(); ++i)
	{
		char name[256];
		sprintf_s(name, "HW/monster_04_alien_%1zu.png", i );
		//sprintf_s(name, "Planets/9/%01zu.png", i + 1);
		mTextureIds[i] = X::LoadTexture(name);
	}

}

void SCVAlien::Update(SCV& agent, float deltaTime)
{

	//if (agent.GetPlayerNetworkId() != 1)
	//{
	//	switch (agent.GetPlayerNetworkId()) {
	//	case 1:
	//		agent.mStateMachine->ChangeState(HouseAnim1::GetName());
	//		break;

	//	case 2:
	//		agent.mStateMachine->ChangeState(HouseAnim2::GetName());
	//		break;

	//	}		
	//}

	if (mAnimTime < X::GetTime()) {
		currentTexture++;
		currentTexture %= mTextureIds.size();
		agent.animId = currentTexture;
		agent.mTextureId = mTextureIds[currentTexture];
		mAnimTime = X::GetTime() + 10 * deltaTime;
	}

	if (agent.playerMonster != 3) {
		if (agent.playerMonster == 0)
			agent.mStateMachine->ChangeState(SCVSkeleton::GetName());
		else if (agent.playerMonster == 1)
			agent.mStateMachine->ChangeState(SCVPumpkinhead::GetName());
		else if (agent.playerMonster == 2)
			agent.mStateMachine->ChangeState(SCVGhost::GetName());
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

