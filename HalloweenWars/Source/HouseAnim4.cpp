#include "HouseAnim4.h"

#include "House.h"
#include "HouseAnim0.h"
#include "HouseAnim1.h"
#include "HouseAnim2.h"
#include "HouseAnim3.h"


void HouseAnim4::Enter(House& agent)
{
	timer = 0.0f;
	currentTexture = -1;
	mAnimTime = 0.0f;

	for (size_t i = 0; i < mTextureIds.size(); ++i)
	{
		char name[256];
		//sprintf_s(name, "Moon/idle%02zu.png", i + 1);
		sprintf_s(name, "Planets/6/%01zu.png", i + 1);
		mTextureIds[i] = X::LoadTexture(name);
	}

}

void HouseAnim4::Update(House& agent, float deltaTime)
{
	if (mAnimTime < X::GetTime()) {
		currentTexture++;
		currentTexture %= mTextureIds.size();
		agent.mTextureId = mTextureIds[currentTexture];
		mAnimTime = X::GetTime() + 20 * deltaTime;
	}

	if (agent.GetPlayerNetworkId() != 4) {
		switch (agent.GetPlayerNetworkId()) {
		case 0:
			agent.mStateMachine->ChangeState(HouseAnim0::GetName());
			break;
		case 1:
			agent.mStateMachine->ChangeState(HouseAnim1::GetName());
			break;
		case 2:
			agent.mStateMachine->ChangeState(HouseAnim2::GetName());
			break;
		case 3:
			agent.mStateMachine->ChangeState(HouseAnim3::GetName());
			break;
		}
	}
}

