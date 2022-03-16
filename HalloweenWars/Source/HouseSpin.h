#pragma once

#include <AI.h>
#include <XEngine.h>

class House;

class HouseSpin : public AI::State<House>
{
public:
	static std::string GetName() { return "HouseSpin"; }

	void Enter(House& agent) override;
	void Update(House& agent, float deltaTime) override;
	void Exit(House& agent) override { }

	void SetPlanetNum(size_t planetId) { planet = planetId; }

	float timer = 0.0f;
	std::array<X::TextureId, 60> mTextureIds;
	std::array<X::TextureId, 120> mTextureIds2;
	size_t planet = 1;
	int currentTexture;
	float mAnimTime;
}; 
