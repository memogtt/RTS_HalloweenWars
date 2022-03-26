#pragma once

#include <XEngine.h>

class House;

class Player
{
public:
	Player(int num) :mPlayerNum(num) {};

	const int GetPercentage() const { return mCurrentPercentage; }

	void IncreasePercentage() { mCurrentPercentage = X::Math::Clamp(mCurrentPercentage += 5, 0, 100); }
	void DecreasePercentage() { mCurrentPercentage = X::Math::Clamp(mCurrentPercentage -= 5, 0, 100); }

	//void SetHouseSelected(int houseId) { mHouseSelected = houseId; }

	House* mHouseSelected2 = nullptr;
	std::shared_ptr<House> mHouseSelected;

	int GetId() { return mPlayerNum; }

	const X::Color GetColor() const { return mColor; }
	void SetColor(X::Color c) { mColor = c; }

	const std::string GetName() const { return mName; }
	void SetName(std::string s) { mName = s; }

	const int GetMonsterNum() const { return mMonsterNum; }
	void SetMonsterNum(int num) { mMonsterNum = num; }

private:

	int mPlayerNum = 0;
	int mCurrentPercentage = 50;
	X::Color mColor = X::Colors::Green;
	std::string mName = "default";
	int mMonsterNum = 0;
	//int mHouseSelected = 0;

};

