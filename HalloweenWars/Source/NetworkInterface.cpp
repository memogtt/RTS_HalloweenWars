#include <iostream>
#include <assert.h>
#include "Address.h"
#include "NetworkInterface.h"

#include "Player.h"
#include "House.h"
#include "SCV.h"


#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#endif


struct PlayerN {
	int id = 0;
	Address clientAddress;

	//virtual ~Player() {};
};

struct HelloUpdate
{
	int idPlayer;
	char name[16];
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

struct BUpdate // used for sending and receiving frame ball updates.
{
	X::Math::Vector2 pos;
	X::Color color;
};

struct AttackUpdate
{
	int idPlayer;
	int idHouseOrigin;
	int idHouseDestination;
	int percentage;
};


NetworkInterface::~NetworkInterface()
{
}

NetworkInterface::NetworkInterface(char* ip, unsigned short port) :
	m_socket(CSocket(ip, port))
{
	m_socket.Open();
}

bool NetworkInterface::sendData(char* msg, int sz)
{
	Address dest;
	return m_socket.Send(dest, msg, sz);
}

bool NetworkInterface::sendDataTo(char* msg, int sz, int playerNumber)
{
	Address dest;
	if (currentPlayers > 0) {
		for (auto& player : mPlayers) {
			if (player.id == playerNumber) {
				m_socket.Send(player.clientAddress, msg, sz);
				break;
			}
		}
	}
	return true;// m_socket.Send(dest, msg, sz);
}

bool NetworkInterface::sendDataBroadcast(const char* msg, int sz)
{
	Address dest;
	if (currentPlayers > 0) {
		for (auto& player : mPlayers) {
			m_socket.Send(player.clientAddress, msg, sz);
		}
	}
	return true;// m_socket.Send(dest, msg, sz);
}

int NetworkInterface::recvData(char* buf, int sz)
{
	Address sender;
	auto bytes = m_socket.Receive(sender, buf, sz);

	if (isServer() && bytes > 0) {
		bool is_new = true;

		char senderAddr[60];
		inet_ntop(sender.m_sockaddress.sin_family, &sender.m_sockaddress.sin_addr, (PSTR)&senderAddr, sizeof(senderAddr));
		//InetNtop(sender.m_sockaddress.sin_family, &sender.m_sockaddress.sin_addr, (PWSTR)&senderAddr, sizeof(senderAddr));
		//inet_ntoa(sender.m_sockaddress.sin_addr)

		for (auto& player : mPlayers) {
			char playerAddr[60];
			inet_ntop(player.clientAddress.m_sockaddress.sin_family, &player.clientAddress.m_sockaddress.sin_addr, (PSTR)&playerAddr, sizeof(playerAddr));
			//InetNtop(player.clientAddress.m_sockaddress.sin_family, &player.clientAddress.m_sockaddress.sin_addr, (PWSTR)&playerAddr, sizeof(playerAddr));

			//if (inet_ntoa(player->clientAddress.m_sockaddress.sin_addr) == inet_ntoa(sender.m_sockaddress.sin_addr) &&
			if (!strcmp(playerAddr, senderAddr) &&
				//player.clientAddress.m_port == sender.m_port)
				ntohs(player.clientAddress.m_sockaddress.sin_port) == ntohs(sender.m_sockaddress.sin_port))
			{
				is_new = false;
				break;
			}
		}

		if (is_new)
		{
			currentPlayers++;
			PlayerN tmp;
			tmp.clientAddress = sender;
			tmp.id = currentPlayers;
			mPlayers.push_back(tmp);
			//auto p = std::make_unique<Player>();
			//p->clientAddress = sender;
			//p->id = currentPlayers;
			//mPlayers.push_back(p);
		}
	}

	return bytes;
}

int NetworkInterface::recvData(char* buf, int sz, int& playerOut)
{
	Address sender;
	auto bytes = m_socket.Receive(sender, buf, sz);

	if (isServer() && bytes > 0) {
		bool is_new = true;

		char senderAddr[60];
		inet_ntop(sender.m_sockaddress.sin_family, &sender.m_sockaddress.sin_addr, (PSTR)&senderAddr, sizeof(senderAddr));
		//InetNtop(sender.m_sockaddress.sin_family, &sender.m_sockaddress.sin_addr, (PWSTR)&senderAddr, sizeof(senderAddr));
		//inet_ntoa(sender.m_sockaddress.sin_addr)

		for (auto& player : mPlayers) {
			char playerAddr[60];
			inet_ntop(player.clientAddress.m_sockaddress.sin_family, &player.clientAddress.m_sockaddress.sin_addr, (PSTR)&playerAddr, sizeof(playerAddr));
			//InetNtop(player.clientAddress.m_sockaddress.sin_family, &player.clientAddress.m_sockaddress.sin_addr, (PWSTR)&playerAddr, sizeof(playerAddr));

			//if (inet_ntoa(player->clientAddress.m_sockaddress.sin_addr) == inet_ntoa(sender.m_sockaddress.sin_addr) &&
			if (!strcmp(playerAddr, senderAddr) &&
				//player.clientAddress.m_port == sender.m_port)
				ntohs(player.clientAddress.m_sockaddress.sin_port) == ntohs(sender.m_sockaddress.sin_port))
			{
				is_new = false;
				playerOut = player.id;
				break;
			}
		}

		if (is_new)
		{
			currentPlayers++;
			PlayerN tmp;
			tmp.clientAddress = sender;
			tmp.id = currentPlayers;
			mPlayers.push_back(tmp);
			playerOut = tmp.id;
			//auto p = std::make_unique<Player>();
			//p->clientAddress = sender;
			//p->id = currentPlayers;
			//mPlayers.push_back(p);
		}
	}

	return bytes;
}

bool NetworkInterface::isServer() const
{
	return m_socket.isServer();
}

bool NetworkInterface::Initialized() const
{
	return m_socket.IsOpen();

}

bool NetworkInterface::ServerLobbyWaitHello(std::vector<std::shared_ptr<Player>>& players, int& current_players, std::string* playerNames, X::Color* playerColors)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ServerLobbyWaitHello()::pCommPort not initialized yet!\n";
		return false;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	int numPlayerSender{ 0 };
	int bytes = this->recvData(buf, 1024, numPlayerSender);
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

		//char tmp[16] = {};
		//sprintf_s(tmp, "%s", buf);

		//aaaaaaaaaaaaa
		HelloUpdate update;
		update.idPlayer = numPlayerSender;
		sprintf_s(update.name, "%s", buf);

		//char playerNumBack[8] = {};
		//sprintf_s(playerNumBack, "%d", numPlayerSender);
		//std::string testa = "this is testing";

		//char buf2[100] = static_cast<char*>(update);

		this->sendDataTo((char*)&update, sizeof(HelloUpdate), numPlayerSender);
		this->sendDataBroadcast((char*)&update, sizeof(HelloUpdate));

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

bool NetworkInterface::ClientLobbySendHello(char* name, int& currentAssignedPlayer, std::string* playerNames)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ClientLobbySendHello()::pCommPort not initialized yet!\n";
		return false;
	}

	std::string msg2{ std::to_string(this->currentPlayers) };

	char initialHello[16] = {};
	//sprintf_s(initialHello, "hello");
	sprintf_s(initialHello, name);
	//std::string testa = "this is testing";
	this->sendData(initialHello, std::strlen(initialHello));


	char buf[1024]{ 0 };

	int numPlayerSender{ 0 };
	int bytes = this->recvData(buf, 1024);

	int max_attemps = 5;
	int current_attemps = 0;
	while (bytes < 0) {
		if (current_attemps < max_attemps) {
			Sleep(100);
			bytes = this->recvData(buf, 1024);
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


// client could receive hello update or Init game update from server
bool NetworkInterface::ClientLobbyReceiveUpdate(int& current_players, std::string* playerNames, std::vector<std::shared_ptr<House>>& houses, AI::AIWorld& world, std::vector<std::shared_ptr<Player>>& players, X::Color* playerColors)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ClientLobbyReceiveUpdate()::pCommPort not initialized yet!\n";
		return false;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	auto helloSize{ sizeof(HelloUpdate) };

	int bytes = this->recvData(buf, 1024);
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

// sends initial update of houses
bool NetworkInterface::ServerLobbySendInitGame(std::vector<std::shared_ptr<House>>& houses)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ServerLobbySendInitGame()::pCommPort not initialized yet!\n";
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
	this->sendDataBroadcast((char*)&houseVector[0], sizeof(HouseInitialUpdate) * houseVector.size());

	return true;
}

bool NetworkInterface::ServerGameSendHouseUpdate(std::vector<std::shared_ptr<House>>& houses)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ServerGameSendHouseUpdate()::pCommPort not initialized yet!\n";
		return true;
	}

	std::vector<HouseUpdate> houseVector;
	for (auto& h : houses) {
		HouseUpdate tmp;
		tmp.idOwner = h->GetPlayerNetworkId();
		tmp.units = h->GetUnits();

		houseVector.push_back(tmp);
	}
	this->sendDataBroadcast((char*)&houseVector[0], sizeof(HouseUpdate) * houseVector.size());

	return true;
}

bool NetworkInterface::ServerGameSendMonsterUpdate(std::vector<std::unique_ptr<SCV>>& scvs)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ServerGameSendMonsterUpdate()::pCommPort not initialized yet!\n";
		return false;
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
			this->sendDataBroadcast((char*)&testin[0], sizeof(BUpdate) * size);
		}
		else
		{
			for (int i = 0; i < loops; ++i)
			{
				//X::DrawScreenText(std::to_string(sizeof(BUpdate) * testscvs.size()).c_str(), 100.0f, 100.0f, 20.f, X::Colors::White);
				if (i == loops)
					this->sendDataBroadcast((char*)&testin[i * count_in_chunk_size], sizeof(BUpdate) * (size - (i * count_in_chunk_size)));
				else
					this->sendDataBroadcast((char*)&testin[i * count_in_chunk_size], sizeof(BUpdate) * count_in_chunk_size);
			}
		}
	}
	return true;
}

bool NetworkInterface::ServerGameReceiveCommand(std::vector<std::shared_ptr<House>>& houses, std::vector<std::unique_ptr<SCV>>& scvs, std::vector<std::shared_ptr<Player>>& players, AI::AIWorld& world)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ServerGameReceiveCommand()::pCommPort not initialized yet!\n";
		return false;
	}

	char buf[1024]{ 0 };
	int bytes = this->recvData(buf, 1024);
	while (bytes > 0)
	{
		auto bSize{ sizeof(AttackUpdate) };
		AttackUpdate* b = reinterpret_cast<AttackUpdate*>(buf);

		houses[b->idHouseOrigin]->sendMonsters(world, houses[b->idHouseDestination], players[b->idPlayer - 1], scvs, b->percentage);
		bytes = this->recvData(buf, 1024);
	}

	return true;
}

bool NetworkInterface::ClientGameReceiveHouseUpdate(std::vector<std::shared_ptr<House>>& houses, std::vector<std::shared_ptr<Player>>& players)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ClientGameReceiveHouseUpdate()::pCommPort not initialized yet!\n";
		return true;
	}

	char buf[1024]{ 0 };
	//char buf[4096]{ 0 };
	//memset(buf, 0, 1024);
	int bytes = this->recvData(buf, 1024);

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
					//X::DrawSprite(textura, b->pos);
					X::DrawScreenCircle({ b->pos,20.0f }, b->color);
				}
			}
		}
		bytes = this->recvData(buf, 1024);
	}
	return true;
}

bool NetworkInterface::ClientGameSendCommand(int idPlayer, int idHouseOrigin, int idHouseDestination, int percentage)
{

	if (!this || !this->Initialized())
	{
		std::cerr << "ClientGameSendCommand()::pCommPort not initialized yet!\n";
		return false;
	}

	AttackUpdate attack;
	attack.idPlayer = idPlayer;
	attack.idHouseOrigin = idHouseOrigin;
	attack.idHouseDestination = idHouseDestination;
	attack.percentage = percentage;

	this->sendData((char*)&attack, sizeof(AttackUpdate));

	return true;
}

