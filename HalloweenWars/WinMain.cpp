#include "Player.h"
#include "House.h"
#include "SCV.h"

#include "NetworkInterface.h"

#include <AI.h>
#include <XEngine.h>
#include <ImGui/Inc/imgui.h>

AI::AIWorld world;
//std::vector<std::unique_ptr<House>> houses;
std::vector<std::shared_ptr<House>> houses;
std::vector<std::unique_ptr<SCV>> scvs;
std::vector<std::shared_ptr<Player>> players;

X::TextureId textura = 0; 

std::unique_ptr<NetworkInterface> pCommPort;
const unsigned short DefaultPort = 8889;

X::Color playerColors[] = {
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

bool waitingClients();
bool sendHello();
bool clientNewPlayersUpdate();

void generateHouses(int num);
void assignCurrentPlayersToHouses(int num);
bool sendInitialHouseUpdate();
bool sendHouseUpdate();
bool receiveHouseUpdate();



void severSendUpdate();
void clientReceiveUpdate();

void clientSendCommand(int idPlayer, int idHouseOrigin, int idHouseDestination, int percentage);
void serverReceiveCommand();


bool receiveUpdatePosition();

bool isServer, isClient;
char ip[16];
char name[16];

std::string playerNames[8];

int current_players = 0;
bool isHelloSent = false;
int numberHousesToGenerate = 20;

struct BUpdate // used for sending and receiving frame ball updates.
{
	X::Math::Vector2 pos;
	X::Color color;
};

struct HouseInitialUpdate
{
	int idOwner;
	X::Math::Circle circle;
	float regen;
	float units;
	int networkId;
};

struct HouseUpdate
{
	int idOwner;
	float units;
};

struct AttackUpdate
{
	int idPlayer;
	int idHouseOrigin;
	int idHouseDestination;
	int percentage;
};

struct HelloUpdate
{
	int idPlayer;
	char name[16];
};




int checkWinCondition() {
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


bool GameLoopInitial(float deltaTime)
{
	static int typeInterface = 0;
	
	ImGui::Begin("Select server or client", nullptr);
	ImGui::SetWindowSize(ImVec2(300.0f, 150.0f));
	ImGui::RadioButton("Server", &typeInterface, 1);
	ImGui::RadioButton("Client", &typeInterface, 2);

	if (typeInterface == 2)
	{
		ImGui::InputText("ip address", ip, 16);
		ImGui::InputText("your name", name, 16);
	}

	if (typeInterface != 0) {
		if (ImGui::Button("Connect")) {

			if (typeInterface == 2)
			{
				//sprintf_s(ip, "127.0.0.1", 16);
				//sprintf_s(ip, "10.0.0.210", 16);
				isClient = true;
				isServer = false;
			}
			else
			{
				sprintf_s(ip, "0");
				isClient = false;
				isServer = true;
			}
			pCommPort = std::make_unique<NetworkInterface>(ip, DefaultPort);
			//pCommPort = new NetworkInterface(ip, DefaultPort);
			ImGui::End();
			return true;
		}
	}
	ImGui::End();

	return false;
}



bool GameLoopLobby(float deltaTime)
{

	if (isServer) {
		X::DrawScreenText("Waiting for clients...", 200.0f, 100.0f, 20.f, X::Colors::White);
		waitingClients();
	}
	else
	{
		if (!isHelloSent)
		{
			isHelloSent = sendHello();

			if (!isHelloSent) {
				if (ImGui::Button("couldnt connect. Click to try again?"))
				{
					//X::Run(GameLoopInitial);
					return true;
				}
			}
		}

		if (currentAssignedPlayer > 0) {

			std::string multiMessage;

			if (pCommPort->isServer())
				multiMessage.append("Server");
			else {
				std::string tmp = { name };
				multiMessage.append("Welcome player " + tmp);
			}
			//multiMessage.append("Welcome player " + std::to_string(currentAssignedPlayer));


			X::DrawScreenText(multiMessage.c_str(), 200.0f, 50.0f, 30.f, playerColors[currentAssignedPlayer]);

			if (clientNewPlayersUpdate())
			{
				world.Initialize({
					(float)X::GetScreenWidth(),
					(float)X::GetScreenHeight()
					});

				assignCurrentPlayersToHouses(current_players);
				currentplayer = players[currentAssignedPlayer - 1];
				return true;
			}
		}
	}

	std::string multiMessage;
	multiMessage.append("Currently " + std::to_string(current_players) + " players:");
	X::DrawScreenText(multiMessage.c_str(), 200.0f, 130.0f, 20.f, X::Colors::White);

	float offsetY = 30.0f;
	for (int i = 1; i < current_players + 1; ++i) {
		multiMessage = "Player " + std::to_string(i) + " = " + playerNames[i];
		X::DrawScreenText(multiMessage.c_str(), 200.0f, 130.0f + offsetY, 20.f, playerColors[i]);
		offsetY += 30.0f;
	}

	if (pCommPort->isServer()) {
		if (current_players > 0) {
			X::DrawScreenText(players[0]->GetName().c_str(), 200.0f, 200.0f, 20.f, playerColors[0]);
		}

		ImGui::Begin("Game configuration", nullptr);
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
				sendInitialHouseUpdate();

				currentplayer = players[current_players - 1];
				ImGui::End();
				return true;
			}
		}
		ImGui::End();
	}

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

void GameInit()
{

	X::DrawScreenText("hi", (float)X::GetScreenWidth() / 2.0f, (float)X::GetScreenHeight() / 2.0f, 80.0f, X::Colors::White);

	world.Initialize({
		(float)X::GetScreenWidth(),
		(float)X::GetScreenHeight()
		});


	generateHouses(20);

	//assignCurrentPlayersToHouses(current_players);

	currentplayer = players.front();
}

void GameCleanup()
{
	pCommPort->m_socket.Close();
}



bool GameLoop(float deltaTime)
{


	if (pCommPort->isServer()) {
		if (players.empty())
			return true;

		for (auto& scv : scvs) {
			int i = 1;
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

		sendHouseUpdate();
		severSendUpdate();
		serverReceiveCommand();

		for (auto& scv : scvs)
		{
			scv->Update(deltaTime);
		}

		for (auto& scv : scvs)
			scv->Render();
	}
	else
	{
		receiveHouseUpdate();
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
						clientSendCommand(currentplayer->GetId(), currentplayer->mHouseSelected->GetNetworkHouseId(), house->GetNetworkHouseId(), currentplayer->GetPercentage());
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

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	X::Start();
	textura = X::LoadTexture("cat_03.png");
	X::Run(GameLoopInitial);
	X::Run(GameLoopLobby);

	if (!pCommPort->isServer()) {
		while (!isHelloSent)
		{
			X::Run(GameLoopInitial);
			X::Run(GameLoopLobby);
		}
	}

	//GameInit();	//for single player
	X::Run(GameLoop);

	GameCleanup();
	X::Stop();
	return 0;
}

bool waitingClients()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "receiveUpdate()::pCommPort not initialized yet!\n";
		return false;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	int numPlayerSender{ 0 };
	int bytes = pCommPort->recvData(buf, 1024, numPlayerSender);
	if (bytes < 0)
	{
		//std::cerr << "Error receiveUpdate()\n";
		//X::DrawScreenText("ERROR recv", 100.0f, 100.0f, 20.f, X::Colors::White);
		return false;
	}
	else
	{
		//buf[bytes] = '\n';

	/*	if (!strcmp(buf, "hello"))
		{*/
		auto& player = players.emplace_back(std::make_shared<Player>(numPlayerSender));
		player->SetColor(playerColors[numPlayerSender]);
		player->SetName(buf);
		//houses[numPlayerSender]->SetOwner(player);

		char tmp[16] = {};
		sprintf_s(tmp, "%s", buf);

		//aaaaaaaaaaaaa
		HelloUpdate update;
		update.idPlayer = numPlayerSender;
		sprintf_s(update.name, "%s", buf);

		char playerNumBack[8] = {};
		sprintf_s(playerNumBack, "%d", numPlayerSender);
		//std::string testa = "this is testing";

		//char buf2[100] = static_cast<char*>(update);

		pCommPort->sendDataTo((char*)&update, sizeof(HelloUpdate), numPlayerSender);
		pCommPort->sendDataBroadcast((char*)&update, sizeof(HelloUpdate));

		//pCommPort->sendDataTo(playerNumBack, std::strlen(playerNumBack), numPlayerSender);
		//pCommPort->sendDataBroadcast(playerNumBack, std::strlen(playerNumBack));

		current_players = numPlayerSender;//last player numberplayer
		playerNames[current_players] = buf;

		return true;
		//}
		//X::DrawScreenText(buf, 100.0f, 100.0f, 20.f, X::Colors::White);
		//current_players = atoi(buf);
	}

	return false;
}

bool sendHello()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "receiveUpdate()::pCommPort not initialized yet!\n";
		return false;
	}

	std::string msg2{ std::to_string(pCommPort->currentPlayers) };

	char initialHello[16] = {};
	//sprintf_s(initialHello, "hello");
	sprintf_s(initialHello, name);
	//std::string testa = "this is testing";
	pCommPort->sendData(initialHello, std::strlen(initialHello));


	char buf[1024]{ 0 };

	int numPlayerSender{ 0 };
	int bytes = pCommPort->recvData(buf, 1024);

	int max_attemps = 5;
	int current_attemps = 0;
	while (bytes < 0) {
		if (current_attemps < max_attemps) {
			Sleep(100);
			bytes = pCommPort->recvData(buf, 1024);
			current_attemps++;
		}
		else
		{
			return false;
		}
	}
	HelloUpdate* helloUp = reinterpret_cast<HelloUpdate*>(buf);
	//current_players = atoi(buf);
	currentAssignedPlayer = helloUp->idPlayer;
	playerNames[currentAssignedPlayer] = helloUp->name;

	//currentAssignedPlayer = atoi(buf);
	return true;
}


bool clientNewPlayersUpdate()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "receiveUpdate()::pCommPort not initialized yet!\n";
		return false;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	auto helloSize{ sizeof(HelloUpdate) };

	int bytes = pCommPort->recvData(buf, 1024);
	if (bytes < 0)
	{
		//std::cerr << "Error receiveUpdate()\n";

		return false;
	}
	//eeee
	//else if (bytes < 8)
	else if (bytes == helloSize)
	{
		//std::cerr << "Error receiveUpdate()\n";
		HelloUpdate* helloUp = reinterpret_cast<HelloUpdate*>(buf);
		//current_players = atoi(buf);
		current_players = helloUp->idPlayer;
		playerNames[current_players] = helloUp->name;

		return false;
	}
	else
	{
		auto bSize{ sizeof(HouseInitialUpdate) };
		int rCount = bytes / bSize;
		//assert(rCount * bSize == bytes);
		//assert(rCount == BallCount);
		if (rCount * bSize == bytes) {
			for (int i = 0; i < rCount; ++i)
			{
				HouseInitialUpdate* b = reinterpret_cast<HouseInitialUpdate*>(buf + i * bSize);

				auto& house = houses.emplace_back(std::make_shared<House>(world));
				house->position = b->circle.center;
				house->radius = b->circle.radius;
				house->SetRegenRate(b->regen);
				house->Initialize();
				house->SetUnits(b->units);
				house->SetNetworkHouseId(b->networkId);
			}


			for (size_t i = 0; i < current_players; ++i)
			{
				auto& player = players.emplace_back(std::make_shared<Player>(i + 1));
				player->SetColor(playerColors[i + 1]);
				player->SetName(playerNames[i]);

				houses[i]->SetOwner(player);
				houses[i]->SetPlayerOwner(i + 1);
			}
			return true;
		}
	}
	return false;
}

void clientReceiveUpdate()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "receiveUpdate()::pCommPort not initialized yet!\n";
		return;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	int bytes = pCommPort->recvData(buf, 1024);
	if (bytes < 0)
	{
		//std::cerr << "Error receiveUpdate()\n";
		X::DrawScreenText("ERROR recv", 100.0f, 100.0f, 20.f, X::Colors::White);
		return;
	}
	else
	{
		buf[bytes] = '\n';
		//X::DrawScreenText(buf, 100.0f, 100.0f, 20.f, X::Colors::White);
		current_players = atoi(buf);
	}
}

void serializeLocalSample(char* buf)
{
	const auto objectSize{ sizeof(Player) };
	for (int i = 0; i < players.size(); ++i)
	{
		memcpy_s(buf + i * objectSize, objectSize, &players[i], objectSize);
	}
}

bool sendInitialHouseUpdate()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "sendUpdate()::pCommPort not initialized yet!\n";
		return true;
	}

	//static HouseUpdate sHouse[2];
	//std::vector<std::unique_ptr<BUpdate>> testin;
	std::vector<HouseInitialUpdate> houseVector;
	for (auto& h : houses) {
		HouseInitialUpdate tmp;
		tmp.idOwner = h->GetPlayerNetworkId();
		tmp.circle = h->getCircle();
		tmp.regen = h->GetRegenRate();
		tmp.units = h->GetUnits();
		tmp.networkId = h->GetNetworkHouseId();
		//auto& bupdate = testin.emplace_back(std::make_unique<BUpdate>());
		//bupdate->pos = scv->position;

		houseVector.push_back(tmp);
	}
	//X::DrawScreenText(std::to_string(sizeof(BUpdate) * testscvs.size()).c_str(), 100.0f, 100.0f, 20.f, X::Colors::White);
	pCommPort->sendDataBroadcast((char*)&houseVector[0], sizeof(HouseInitialUpdate) * houseVector.size());

	return true;
}

bool sendHouseUpdate()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "sendUpdate()::pCommPort not initialized yet!\n";
		return true;
	}

	std::vector<HouseUpdate> houseVector;
	for (auto& h : houses) {
		HouseUpdate tmp;
		tmp.idOwner = h->GetPlayerNetworkId();
		tmp.units = h->GetUnits();

		houseVector.push_back(tmp);
	}
	pCommPort->sendDataBroadcast((char*)&houseVector[0], sizeof(HouseUpdate) * houseVector.size());

	return true;
}

bool receiveHouseUpdate()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "sendUpdate()::pCommPort not initialized yet!\n";
		return true;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	int bytes = pCommPort->recvData(buf, 1024);

	while (bytes > 0) {
		if (bytes < 0)
		{
			//std::cerr << "Error receiveUpdate()\n";
			return false;
		}
		else
		{
			auto hSize{ sizeof(HouseUpdate) };
			int hCount = bytes / hSize;
			assert(hCount * hSize == bytes);
			//assert(rCount == houses.size())
			if (hCount == houses.size()) {
				for (int i = 0; i < hCount; ++i)
				{
					HouseUpdate* b = reinterpret_cast<HouseUpdate*>(buf + i * hSize);

					houses[i]->SetUnits(b->units);
					houses[i]->SetPlayerOwner(b->idOwner);
					if ((b->idOwner) != 0)
						houses[i]->SetOwner(players[b->idOwner - 1]);
				}
			}
			else
			{
				auto bSize{ sizeof(BUpdate) };
				int rCount = bytes / bSize;
				assert(rCount * bSize == bytes);
				//assert(rCount == BallCount);
				for (int i = 0; i < rCount; ++i)
				{
					BUpdate* b = reinterpret_cast<BUpdate*>(buf + i * bSize);
					X::DrawSprite(textura, b->pos);
					X::DrawScreenCircle({ b->pos,20.0f }, b->color);
				}
			}
		}
		bytes = pCommPort->recvData(buf, 1024);
	}
	return true;
}

void severSendUpdate()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "sendUpdate()::pCommPort not initialized yet!\n";
		return;
	}

	if (!scvs.empty()) {
		int size = scvs.size();

		auto hSize{ sizeof(BUpdate) };
		int loops = ceil((hSize * static_cast<float>(size)) / 800.0f);
		int count_in_chunk_size = 800 / hSize;
		//assert(hCount * hSize == bytes);
		//assert(rCount == houses.size())

		std::vector<BUpdate> testin;
		for (auto& scv : scvs) {
			BUpdate tmp;
			tmp.pos = scv->position;
			//tmp.dest = scv->destination;
			tmp.color = scv->GetOwner()->GetColor();

			testin.push_back(tmp);
		}
		if (loops == 1)
		{
			pCommPort->sendDataBroadcast((char*)&testin[0], sizeof(BUpdate) * size);
		}
		else
		{
			for (int i = 0; i < loops; ++i)
			{
				//X::DrawScreenText(std::to_string(sizeof(BUpdate) * testscvs.size()).c_str(), 100.0f, 100.0f, 20.f, X::Colors::White);
				if (i == loops)
					pCommPort->sendDataBroadcast((char*)&testin[i * count_in_chunk_size], sizeof(BUpdate) * (size - (i * count_in_chunk_size)));
				else
					pCommPort->sendDataBroadcast((char*)&testin[i * count_in_chunk_size], sizeof(BUpdate) * count_in_chunk_size);
			}
		}
	}
}

void clientSendCommand(int idPlayer, int idHouseOrigin, int idHouseDestination, int percentage)
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "clientSendCommand()::pCommPort not initialized yet!\n";
		return;
	}

	AttackUpdate attack;
	attack.idPlayer = idPlayer;
	attack.idHouseOrigin = idHouseOrigin;
	attack.idHouseDestination = idHouseDestination;
	attack.percentage = percentage;

	pCommPort->sendData((char*)&attack, sizeof(AttackUpdate));

}
void serverReceiveCommand()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "serverReceiveCommand()::pCommPort not initialized yet!\n";
		return;
	}

	char buf[1024]{ 0 };
	int bytes = pCommPort->recvData(buf, 1024);
	while (bytes > 0)
	{
		auto bSize{ sizeof(AttackUpdate) };
		AttackUpdate* b = reinterpret_cast<AttackUpdate*>(buf);

		houses[b->idHouseOrigin]->sendMonsters(world, houses[b->idHouseDestination], players[b->idPlayer - 1], scvs, b->percentage);
		bytes = pCommPort->recvData(buf, 1024);
	}

	return;
}


bool receiveUpdatePosition()
{
	if (!pCommPort || !pCommPort->Initialized())
	{
		std::cerr << "receiveUpdate()::pCommPort not initialized yet!\n";
		return false;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	int bytes = pCommPort->recvData(buf, 1024);
	if (bytes < 0)
	{
		//std::cerr << "Error receiveUpdate()\n";
		X::DrawScreenText("ERROR", 100.0f, 100.0f, 20.f, X::Colors::White);
		return false;
	}
	else
	{
		auto bSize{ sizeof(BUpdate) };
		int rCount = bytes / bSize;
		assert(rCount * bSize == bytes);
		//assert(rCount == BallCount);
		for (int i = 0; i < rCount; ++i)
		{
			BUpdate* b = reinterpret_cast<BUpdate*>(buf + i * bSize);
			X::DrawSprite(textura, b->pos);
			X::DrawScreenCircle({ b->pos,20.0f }, b->color);
			
		}

		return true;
	}
}

void generateHouses(int num) {

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
		house->Initialize();
		house->SetUnits(X::RandomFloat(0.0f, 100.0f));
		house->SetNetworkHouseId(i);

	}
}

void assignCurrentPlayersToHouses(int num)
{

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