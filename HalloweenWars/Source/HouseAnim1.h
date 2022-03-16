#pragma once

#include <AI.h>
#include <XEngine.h>

class House;

class HouseAnim1 : public AI::State<House>
{
public:
	static std::string GetName() { return "HouseAnim1"; }

	void Enter(House& agent) override;
	void Update(House& agent, float deltaTime) override;
	void Exit(House& agent) override { }

	float timer = 0.0f;
	std::array<X::TextureId, 120> mTextureIds;
	
	int currentTexture;
	float mAnimTime;
};
