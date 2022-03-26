#pragma once

#include <AI.h>
#include <XEngine.h>

class SCV;

class SCVSkeleton : public AI::State<SCV>
{
public:
	static std::string GetName() { return "SCVSkeleton"; }

	void Enter(SCV& agent) override;
	void Update(SCV& agent, float deltaTime) override;
	void Exit(SCV& agent) override { }

	float timer = 0.0f;
	std::array<X::TextureId, 3> mTextureIds;
	
	int currentTexture;
	float mAnimTime;
};
