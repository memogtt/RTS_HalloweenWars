#include "GameEngine.h"

// Engine headers
#include <AI.h>
#include <XEngine.h>
#include <ImGui/Inc/imgui.h>

// My game headers
#include "Player.h"
#include "House.h"
#include "SCV.h"
#include "NetworkInterface.h"

using namespace Core;

AI::AIWorld world;
//std::vector<std::unique_ptr<House>> houses;
std::vector<std::shared_ptr<House>> houses;
std::vector<std::unique_ptr<SCV>> scvs;
std::vector<std::shared_ptr<Player>> players;

X::TextureId textura = 0;

std::unique_ptr<NetworkInterface> pCommPort;
const unsigned short DefaultPort = 8889;

std::array<X::Color, 9> playerColors2{
	X::Colors::White,
	X::Colors::Blue,
	X::Colors::Red,
	X::Colors::Green,
	X::Colors::Yellow,
	X::Colors::Pink,
	X::Colors::Orange,
	X::Colors::Turquoise,
	X::Colors::Violet
};

X::Color playerColors[9] = {
   X::Colors::White,
   X::Colors::Blue,
   X::Colors::Red,
   X::Colors::Green,
   X::Colors::Yellow,
   X::Colors::Pink,
   X::Colors::Orange,
   X::Colors::Turquoise,
   X::Colors::Violet
};

bool seekActive, arriveActive, wanderActive, collisionActive, separationActive, aligmentActive, cohesionActive;
float scroll = 0.0f;

std::shared_ptr<Player> player(std::make_shared<Player>(2));
std::shared_ptr<Player> currentplayer;
int currentAssignedPlayer = 0;

char ip[16];
char name[16];

bool gameStarted = false;

int current_players = 0;
std::string playerNames[8];
std::array<int, 8> playerMonster;
//int playerMonster[8];

int numberHousesToGenerate = 20;

static int current_monster = 0;
std::array<X::TextureId, 12> mTextureIds;

void GameEngine::GameStart()
{
	X::Start();

	mTextureIds[0] = X::LoadTexture("HW/monster_01_skeleton_0.png");
	mTextureIds[1] = X::LoadTexture("HW/monster_02_pumpkinhead_0.png");
	mTextureIds[2] = X::LoadTexture("HW/monster_03_ghost_0.png");
	mTextureIds[3] = X::LoadTexture("HW/monster_04_alien_0.png");
	
	mTextureIds[4] = X::LoadTexture("HW/monster_01_skeleton_1.png");
	mTextureIds[5] = X::LoadTexture("HW/monster_02_pumpkinhead_1.png");
	mTextureIds[6] = X::LoadTexture("HW/monster_03_ghost_1.png");
	mTextureIds[7] = X::LoadTexture("HW/monster_04_alien_1.png");

	mTextureIds[8] = X::LoadTexture("HW/monster_01_skeleton_2.png");
	mTextureIds[9] = X::LoadTexture("HW/monster_02_pumpkinhead_2.png");
	mTextureIds[10] = X::LoadTexture("HW/monster_03_ghost_2.png");
	mTextureIds[11] = X::LoadTexture("HW/monster_04_alien_2.png");
}

void GameEngine::GameLoop()
{
	//X::Run(GameLoopInitialSinglePlayer);
	//X::Run(GameLoopSinglePlayer);

	X::Run(GameLoopInitial);

	if (pCommPort != nullptr) {
		X::Run(GameLoopLobby);

		//GameInitSinglePlayer();	//for single player
		if (gameStarted)
			X::Run(GameLoopMultiPlayer);
	}
}

void GameEngine::GameEnd()
{
	GameCleanup();
	X::Stop();
}

bool GameEngine::GameLoopInitial(float deltaTime)
{
	static int typeInterface = 0;

	//ImGui::SetNextWindowPos()
	ImGui::SetNextWindowSize(ImVec2(300.0f, 150.0f));
	ImGui::Begin("Select server or client", nullptr);
	ImGui::RadioButton("Server", &typeInterface, 1);
	ImGui::RadioButton("Client", &typeInterface, 2);

	if (typeInterface == 2)
	{
		ImGui::InputText("ip address", ip, 16);
		ImGui::InputText("your name", name, 16);
	}
	//if (typeInterface == 1 || (typeInterface == 2 && ip[0] != '\0' && name[0] != '\0')) {
	//if (typeInterface == 1 || (typeInterface == 2 && name[0] != '\0')) {

	if (ImGui::Button("Connect")) {

		if (typeInterface == 2)
		{
			//sprintf_s(name, "memo", 16);
			//sprintf_s(ip, "127.0.0.1", 16);
			//sprintf_s(ip, "10.0.0.210", 16);
		}
		else
		{
			sprintf_s(ip, "0");
		}

		//if (!pCommPort)
		pCommPort = std::make_unique<NetworkInterface>(ip, DefaultPort);

		ImGui::End();

		if (!pCommPort->isServer())
			if (!pCommPort->ClientLobbySendHello(name, currentAssignedPlayer, playerNames))
				return false;

		return true;
	}
	//}
	ImGui::End();

	return X::IsKeyPressed(X::Keys::ESCAPE);
}

bool GameEngine::GameLoopLobby(float deltaTime)
{
	ImGui::Begin("Game configuration", nullptr);
	const char* monsterTypes[] = { "Skeleton", "Pumpkinhead", "Ghost", "Alien" };

	if (pCommPort->isServer()) {
		X::DrawScreenText("Waiting for clients...", 200.0f, 100.0f, 20.f, X::Colors::White);
		pCommPort->ServerLobbyWaitHello(players, current_players, playerNames, playerColors, playerMonster);
	}
	else
	{
		if (currentAssignedPlayer > 0) {

			std::string multiMessage;
			multiMessage.append("Welcome player " + std::string(name));

			X::DrawScreenText(multiMessage.c_str(), 200.0f, 50.0f, 30.f, playerColors[currentAssignedPlayer]);




			int previuosMonster = current_monster;
			ImGui::Combo("Monster type", &current_monster, monsterTypes, IM_ARRAYSIZE(monsterTypes));

			if (previuosMonster != current_monster)
			{
				pCommPort->ClientLobbyUpdateMonsterType(currentAssignedPlayer, current_monster);
			}


			if (pCommPort->ClientLobbyReceiveUpdate(current_players, playerNames, houses, world, players, playerColors, playerMonster))
			{
				world.Initialize({
					(float)X::GetScreenWidth(),
					(float)X::GetScreenHeight()
					});

				assignCurrentPlayersToHouses(current_players);
				currentplayer = players[currentAssignedPlayer - 1];
				ImGui::End();
				gameStarted = true;
				return true;
			}
		}
	}

	std::string multiMessage;
	multiMessage.append("Currently " + std::to_string(current_players) + " players:");
	X::DrawScreenText(multiMessage.c_str(), 200.0f, 130.0f, 20.f, X::Colors::White);

	float offsetY = 30.0f;
	for (int i = 1; i < current_players + 1; ++i) {
		multiMessage = "Player " + std::to_string(i) + " = " + playerNames[i] + " Monster Type: " + monsterTypes[playerMonster[i]];
		X::DrawScreenText(multiMessage.c_str(), 200.0f, 130.0f + offsetY, 20.f, playerColors[i]);
		offsetY += 30.0f;
	}

	if (pCommPort->isServer()) {
		if (current_players > 0) {
			//X::DrawScreenText(players[0]->GetName().c_str(), 200.0f, 200.0f, 20.f, playerColors[0]);
		}

		//		ImGui::Begin("Game configuration", nullptr);
		ImGui::SetWindowSize(ImVec2(300.0f, 150.0f));
		ImGui::SliderInt("House number", &numberHousesToGenerate, 5, 30);
		if (ImGui::Button("StartGame"))
		{
			if (current_players > 0) {
				world.Initialize({
				(float)X::GetScreenWidth(),
				(float)X::GetScreenHeight()
					});

				generateHouses(numberHousesToGenerate);
				assignCurrentPlayersToHouses(current_players);
				pCommPort->ServerLobbySendInitGame(houses);

				currentplayer = players[current_players - 1];
				ImGui::End();
				gameStarted = true;
				return true;
			}
		}
		//		ImGui::End();
	}
	ImGui::End();
	////dinamyc way to load them
	//std::string multiMessage;
	//multiMessage.append("Currently " + std::to_string(players.size()) + " players:");
	//X::DrawScreenText(multiMessage.c_str(), 200.0f, 130.0f, 20.f, X::Colors::White);

	//float offsetY = 30.0f;
	//for (auto& p : players) {
	//	multiMessage = "Player " + std::to_string(p->GetId());
	//	X::DrawScreenText(multiMessage.c_str(), 200.0f, 130.0f + offsetY, 20.f, p->GetColor());
	//	offsetY += 30.0f;
	//}

	return X::IsKeyPressed(X::Keys::ESCAPE);
}

bool GameEngine::GameLoopInitialSinglePlayer(float deltaTime)
{
	X::DrawScreenText("hi", (float)X::GetScreenWidth() / 2.0f, (float)X::GetScreenHeight() / 2.0f, 80.0f, X::Colors::White);

	world.Initialize({
		(float)X::GetScreenWidth(),
		(float)X::GetScreenHeight()
		});


	generateHouses(20);
	assignCurrentPlayersToHouses(2,true);
	currentplayer = players.front();
	//assignCurrentPlayersToHouses(current_players);

	//currentplayer = players.front();

	return true;
}

bool GameEngine::GameLoopMultiPlayer(float deltaTime)
{

	if (pCommPort->isServer()) {
		if (players.empty())
			return true;

		for (auto& scv : scvs) {
			if (scv->checkCollision())
			{
				auto iter = std::find(scvs.begin(), scvs.end(), scv);
				if (iter != scvs.end())
				{
					std::iter_swap(iter, scvs.end() - 1);
					scvs.pop_back();
					break;
				}
			}
		}

		world.Update();

		pCommPort->ServerGameSendHouseUpdate(houses);
		pCommPort->ServerGameSendMonsterUpdate(scvs);
		pCommPort->ServerGameReceiveCommand(houses, scvs, players, world);

		for (auto& scv : scvs)
		{
			scv->Update(deltaTime);
		}

		for (auto& scv : scvs)
			scv->Render();
	}
	else
	{
		pCommPort->ClientGameReceiveHouseUpdate(houses, players, mTextureIds, playerMonster, playerColors2);
	}


	for (auto& house : houses) {
		house->Update(deltaTime);
		house->Render();
	}

	if (X::IsMousePressed(X::Mouse::LBUTTON))
	{
		for (auto& house : houses) {
			if (house->GetPlayerOwner() == currentplayer->GetId() && house->mouseOver({ (float)X::GetMouseScreenX(),(float)X::GetMouseScreenY() }))
			{
				currentplayer->mHouseSelected = house;
				break;
			}
		}
	}

	if (currentplayer->mHouseSelected != nullptr && currentplayer->mHouseSelected->GetPlayerOwner() == currentplayer->GetId()) {
		//X::DrawScreenText(std::to_string(currentplayer->mHouseSelected->id).c_str(), 200.0f, (float)X::GetScreenHeight() - 40.0f, 20.0f, X::Colors::White);
		X::DrawScreenCircle(currentplayer->mHouseSelected->position, currentplayer->mHouseSelected->radius + 1, currentplayer->GetColor());
		X::DrawScreenCircle(currentplayer->mHouseSelected->position, currentplayer->mHouseSelected->radius + 2, currentplayer->GetColor());
		X::DrawScreenCircle(currentplayer->mHouseSelected->position, currentplayer->mHouseSelected->radius + 3, currentplayer->GetColor());
	}
	else
	{
		//X::DrawScreenText("NONE SELECTED", 200.0f, (float)X::GetScreenHeight() - 40.0f, 20.0f, X::Colors::White);
		currentplayer->mHouseSelected = nullptr;
	}

	for (auto& house : houses) {
		if (house->mouseOver({ (float)X::GetMouseScreenX(),(float)X::GetMouseScreenY() }))
		{
			X::DrawScreenCircle(house->position, house->radius + 1, currentplayer->GetColor());
			X::DrawScreenCircle(house->position, house->radius + 2, currentplayer->GetColor());
			X::DrawScreenCircle(house->position, house->radius + 3, currentplayer->GetColor());
			if (currentplayer->mHouseSelected != nullptr)
			{
				X::DrawScreenLine(currentplayer->mHouseSelected->position, house->position, currentplayer->GetColor());
			}
			break;
		}
	}

	if (X::IsMousePressed(X::Mouse::RBUTTON))
	{
		if (currentplayer->mHouseSelected != nullptr) {
			for (auto& house : houses) {
				if (house->mouseOver({ (float)X::GetMouseScreenX(),(float)X::GetMouseScreenY() }))
				{
					if (pCommPort->isServer()) {
						currentplayer->mHouseSelected->sendMonsters(world, house, scvs);
					}
					else
					{
						pCommPort->ClientGameSendCommand(currentplayer->GetId(), currentplayer->mHouseSelected->GetNetworkHouseId(), house->GetNetworkHouseId(), currentplayer->GetPercentage());
					}

					break;
				}
			}


		}

	}

	if (scroll != X::GetMouseMoveZ())
	{
		if (scroll < X::GetMouseMoveZ())
		{
			currentplayer->IncreasePercentage();
		}
		else
		{
			currentplayer->DecreasePercentage();
		}
		scroll = X::GetMouseMoveZ();
	}

	char currentPercentage[24];
	sprintf_s(currentPercentage, "Monsters %d %%", currentplayer->GetPercentage());
	//X::DrawScreenText(std::to_string(currentplayer->GetPercentage()).c_str(), 50.0f, (float)X::GetScreenHeight() - 40.0f, 30.0f, X::Colors::White);
	X::DrawScreenText(currentPercentage, 50.0f, (float)X::GetScreenHeight() - 40.0f, 30.0f, X::Colors::White);

	//if (X::IsKeyDown(X::Keys::ONE))
	//{
	//	currentplayer = players[0];
	//}

	//if (X::IsKeyDown(X::Keys::TWO))
	//{
	//	currentplayer = players[1];
	//}

	int playerWin = checkWinCondition();
	if (playerWin != 0)
	{
		std::string multiMessage;
		multiMessage.append("Player " + std::to_string(playerWin) + " WINS!!");
		X::DrawScreenText(multiMessage.c_str(), (float)X::GetScreenWidth() / 4.0f, (float)X::GetScreenHeight() / 2.0f, 80.0f, players[playerWin - 1]->GetColor());
	}

	return X::IsKeyPressed(X::Keys::ESCAPE);
}


bool GameEngine::GameLoopSinglePlayer(float deltaTime)
{

	for (auto& scv : scvs) {
		if (scv->checkCollision())
		{
			auto iter = std::find(scvs.begin(), scvs.end(), scv);
			if (iter != scvs.end())
			{
				std::iter_swap(iter, scvs.end() - 1);
				scvs.pop_back();
				break;
			}
		}
	}

	world.Update();

	for (auto& scv : scvs)
	{
		scv->Update(deltaTime);
	}


	//for (auto& scv : scvs)
	//	X::DrawSprite(mTextureIds[playerMonster[scv->GetOwner()->GetId()]], scv->position);

	for (auto& house : houses) {
		house->Update(deltaTime);
		house->Render();
	}

	for (auto& scv : scvs)
		scv->Render();

	if (X::IsMousePressed(X::Mouse::LBUTTON))
	{
		for (auto& house : houses) {
			if (house->GetPlayerOwner() == currentplayer->GetId() && house->mouseOver({ (float)X::GetMouseScreenX(),(float)X::GetMouseScreenY() }))
			{
				currentplayer->mHouseSelected = house;
				break;
			}
		}
	}

	if (currentplayer->mHouseSelected != nullptr && currentplayer->mHouseSelected->GetPlayerOwner() == currentplayer->GetId()) {
		//X::DrawScreenText(std::to_string(currentplayer->mHouseSelected->id).c_str(), 200.0f, (float)X::GetScreenHeight() - 40.0f, 20.0f, X::Colors::White);
		X::DrawScreenCircle(currentplayer->mHouseSelected->position, currentplayer->mHouseSelected->radius + 1, currentplayer->GetColor());
		X::DrawScreenCircle(currentplayer->mHouseSelected->position, currentplayer->mHouseSelected->radius + 2, currentplayer->GetColor());
		X::DrawScreenCircle(currentplayer->mHouseSelected->position, currentplayer->mHouseSelected->radius + 3, currentplayer->GetColor());
	}
	else
	{
		//X::DrawScreenText("NONE SELECTED", 200.0f, (float)X::GetScreenHeight() - 40.0f, 20.0f, X::Colors::White);
		currentplayer->mHouseSelected = nullptr;
	}

	for (auto& house : houses) {
		if (house->mouseOver({ (float)X::GetMouseScreenX(),(float)X::GetMouseScreenY() }))
		{
			X::DrawScreenCircle(house->position, house->radius + 1, currentplayer->GetColor());
			X::DrawScreenCircle(house->position, house->radius + 2, currentplayer->GetColor());
			X::DrawScreenCircle(house->position, house->radius + 3, currentplayer->GetColor());
			if (currentplayer->mHouseSelected != nullptr)
			{
				X::DrawScreenLine(currentplayer->mHouseSelected->position, house->position, currentplayer->GetColor());
			}
			break;
		}
	}

	if (X::IsMousePressed(X::Mouse::RBUTTON))
	{
		if (currentplayer->mHouseSelected != nullptr) {
			for (auto& house : houses) {
				if (house->mouseOver({ (float)X::GetMouseScreenX(),(float)X::GetMouseScreenY() }))
				{

					currentplayer->mHouseSelected->sendMonsters(world, house, scvs);


					break;
				}
			}
		}
	}

	if (X::IsKeyPressed(X::Keys::ONE))
	{
		currentplayer = players[0];
	}

	if (X::IsKeyPressed(X::Keys::TWO))
	{
		currentplayer = players[1];
	}

	if (scroll != X::GetMouseMoveZ())
	{
		if (scroll < X::GetMouseMoveZ())
		{
			currentplayer->IncreasePercentage();
		}
		else
		{
			currentplayer->DecreasePercentage();
		}
		scroll = X::GetMouseMoveZ();
	}

	char currentPercentage[24];
	sprintf_s(currentPercentage, "Monsters %d %%", currentplayer->GetPercentage());
	//X::DrawScreenText(std::to_string(currentplayer->GetPercentage()).c_str(), 50.0f, (float)X::GetScreenHeight() - 40.0f, 30.0f, X::Colors::White);
	X::DrawScreenText(currentPercentage, 50.0f, (float)X::GetScreenHeight() - 40.0f, 30.0f, X::Colors::White);

	//if (X::IsKeyDown(X::Keys::ONE))
	//{
	//	currentplayer = players[0];
	//}

	//if (X::IsKeyDown(X::Keys::TWO))
	//{
	//	currentplayer = players[1];
	//}

	int playerWin = checkWinCondition();
	if (playerWin != 0)
	{
		std::string multiMessage;
		multiMessage.append("Player " + std::to_string(playerWin) + " WINS!!");
		X::DrawScreenText(multiMessage.c_str(), (float)X::GetScreenWidth() / 4.0f, (float)X::GetScreenHeight() / 2.0f, 80.0f, players[playerWin - 1]->GetColor());
	}

	return X::IsKeyPressed(X::Keys::ESCAPE);
}

bool GameEngine::GameCleanup()
{
	if (pCommPort != nullptr)
		pCommPort->m_socket.Close();
	return true;
}

int GameEngine::checkWinCondition()
{
	int numPlayers = players.size();
	std::vector<int> win;
	for (int i = 0; i < numPlayers; ++i) {
		win.push_back(0);
	}

	int ocuppiedHouses = 0;
	for (auto& house : houses) {

		if (house->GetOwner() != nullptr)
		{
			int tmp = house->GetOwner()->GetId();
			if (tmp != 0)
			{
				win[tmp - 1] += 1;
				ocuppiedHouses++;
			}
		}
	}

	for (int i = 0; i < numPlayers; ++i) {
		if (ocuppiedHouses == win[i])
		{
			return i + 1;
		}
	}

	return 0;
}

void GameEngine::generateHouses(int num)
{
	for (size_t i = 0; i < num; ++i)
	{
		X::Math::Vector2 housePosition{ X::Math::Vector2::Zero() };
		float houseRadius{ X::RandomFloat(20.0f, 64.0f) };

		housePosition.x = X::RandomFloat(64.0f, (float)X::GetScreenWidth() - 64.0f);
		housePosition.y = X::RandomFloat(64.0f, (float)X::GetScreenHeight() - 64.0f);

		// logic so house planet generation doesnt overlap TODO: optimize
		bool loop = true;
		float extraOffsetSeparation = 20.0f;
		while (loop) {
			if (houses.empty())
				break;

			for (auto& check : houses) {
				if (X::Math::PointInCircle(housePosition, { check->position,check->radius + houseRadius + extraOffsetSeparation }))
				{
					housePosition.x = X::RandomFloat(64.0f, (float)X::GetScreenWidth() - 64.0f);
					housePosition.y = X::RandomFloat(64.0f, (float)X::GetScreenHeight() - 64.0f);
					loop = true;
					break;
				}
				else
				{
					loop = false;
				}
			}
		}

		auto& house = houses.emplace_back(std::make_shared<House>(world));
		house->position = housePosition;
		house->radius = houseRadius;
		house->SetRegenRate(X::RandomFloat(0.0f, 2.0f));
		house->SetUnits(X::RandomFloat(0.0f, 100.0f));
		house->SetNetworkHouseId(i);
		house->Initialize();

	}
}

void GameEngine::assignCurrentPlayersToHouses(int num, bool singlePlayer)
{
	if (singlePlayer) {
		for (size_t i = 0; i < num; ++i)
		{
			auto& player = players.emplace_back(std::make_shared<Player>(i + 1));
			//players.emplace_back(std::make_shared<Player>(numPlayerSender));
			player->SetColor(playerColors[i + 1]);
			player->SetName("a");

		}
	}

	for (size_t i = 0; i < num; ++i)
	{
		//auto& player = players.emplace_back(std::make_shared<Player>(i + 1));

		players[i]->SetColor(playerColors[i + 1]);
		houses[i]->SetOwner(players[i]);
		houses[i]->SetPlayerOwner(players[i]->GetId());
		houses[i]->SetUnits(100);
		houses[i]->SetRegenRate(2.0);
	}
}