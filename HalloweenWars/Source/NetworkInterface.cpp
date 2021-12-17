#include <iostream>
#include <assert.h>
#include "Address.h"
#include "NetworkInterface.h"

#include "Player.h"
#include "House.h"
#include "SCV.h"

#include <XEngine.h>

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#endif


struct Header
{
	int id;
	int qty;
};

struct ClientInitialHello
{
	int idPlayer;
	char name[16];
	int idMonster;
};

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
	//X::Color color;
	int ownerId;
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

bool NetworkInterface::ServerLobbyWaitHello(std::vector<std::shared_ptr<Player>>& players, int& current_players, std::string* playerNames, X::Color* playerColors, std::array<int, 8>& playerMonster)
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
		Header* header = reinterpret_cast<Header*>(buf);

		ClientInitialHello* clientHello = reinterpret_cast<ClientInitialHello*>(buf + sizeof(Header));

		if (header->id == 1)
		{
			auto& player = players.emplace_back(std::make_shared<Player>(numPlayerSender));
			player->SetColor(playerColors[numPlayerSender]);
			player->SetName(clientHello->name);

			char bufReply[64]{ 0 };

			Header header;
			header.id = 1;
			header.qty = 1;
			const auto headerSize{ sizeof(Header) };
			memcpy_s(bufReply, headerSize, &header, headerSize);

			ClientInitialHello hello;
			hello.idPlayer = numPlayerSender;
			//strcpy_s(hello.name, name);
			const auto clientInitialHelloSize{ sizeof(ClientInitialHello) };
			memcpy_s(bufReply + headerSize, clientInitialHelloSize, &hello, clientInitialHelloSize);

			this->sendDataTo(bufReply, sizeof(bufReply), hello.idPlayer);

			//HelloUpdate update;
			//update.idPlayer = numPlayerSender;
			//sprintf_s(update.name, "%s", clientHello->name);
			//this->sendDataTo((char*)&update, sizeof(HelloUpdate), numPlayerSender);
			//this->sendDataBroadcast((char*)&update, sizeof(HelloUpdate));

			//current_players = numPlayerSender;//last player numberplayer
			current_players = players.size();//last player numberplayer
			playerNames[hello.idPlayer] = clientHello->name;

		}

		if (header->id == 3)
		{
			playerMonster[clientHello->idPlayer] = clientHello->idMonster;
		}


		char bufPlayers[512]{ 0 };

		Header headerPlayersUpdate;
		headerPlayersUpdate.id = 2;
		headerPlayersUpdate.qty = players.size();

		const auto headerSize{ sizeof(Header) };
		memcpy_s(bufPlayers, headerSize, &headerPlayersUpdate, headerSize);


		const auto objectSize{ sizeof(ClientInitialHello) };
		//const auto objectSize{ sizeof(Player) };
		for (int i = 0; i < players.size(); ++i)
		{
			ClientInitialHello hello;
			hello.idPlayer = players[i]->GetId();
			strcpy_s(hello.name, players[i]->GetName().c_str());
			hello.idMonster = playerMonster[i + 1];

			const auto clientInitialHelloSize{ sizeof(ClientInitialHello) };
			//memcpy_s(bufPlayers + headerSize, clientInitialHelloSize, &hello, clientInitialHelloSize);

			memcpy_s(bufPlayers + headerSize + (i * objectSize), objectSize, &hello, objectSize);
			//memcpy_s(bufPlayers + headerSize + (i * objectSize), objectSize, &players[i], objectSize);
		}

		this->sendDataBroadcast(bufPlayers, sizeof(bufPlayers));

		return true;
		/*	if (!strcmp(buf, "hello"))
			{*/



			////auto& player = players.emplace_back(std::make_shared<Player>(numPlayerSender));
			////player->SetColor(playerColors[numPlayerSender]);
			////player->SetName(clientHello->name);
			//////houses[numPlayerSender]->SetOwner(player);

			//////char tmp[16] = {};
			//////sprintf_s(tmp, "%s", buf);

			////HelloUpdate update;
			////update.idPlayer = numPlayerSender;
			////sprintf_s(update.name, "%s", clientHello->name);

			//////char playerNumBack[8] = {};
			//////sprintf_s(playerNumBack, "%d", numPlayerSender);
			//////std::string testa = "this is testing";

			//////char buf2[100] = static_cast<char*>(update);

			////this->sendDataTo((char*)&update, sizeof(HelloUpdate), numPlayerSender);
			////this->sendDataBroadcast((char*)&update, sizeof(HelloUpdate));

			//////pCommPort->sendDataTo(playerNumBack, std::strlen(playerNumBack), numPlayerSender);
			//////pCommPort->sendDataBroadcast(playerNumBack, std::strlen(playerNumBack));

			////current_players = numPlayerSender;//last player numberplayer
			////playerNames[current_players] = clientHello->name;

			////return true;



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

	char buftest[64]{ 0 };

	Header header;
	header.id = 1;
	header.qty = 1;
	const auto headerSize{ sizeof(Header) };
	memcpy_s(buftest, headerSize, &header, headerSize);

	ClientInitialHello hello;
	hello.idPlayer = 0;
	strcpy_s(hello.name, name);
	const auto clientInitialHelloSize{ sizeof(ClientInitialHello) };
	memcpy_s(buftest + headerSize, clientInitialHelloSize, &hello, clientInitialHelloSize);

	//memcpy_s(buftest + headerSize, sizeof(name), &name, sizeof(name));
	this->sendData(buftest, sizeof(buftest));
	//this->sendData(initialHello, std::strlen(initialHello));


	//const auto objectSize{ sizeof(Player) };
	//for (int i = 0; i < players.size(); ++i)
	//{
	//	memcpy_s(buf + headerSize + (i * objectSize), objectSize, &players[i], objectSize);
	//}


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

	Header* headerReply = reinterpret_cast<Header*>(buf);

	ClientInitialHello* clientHello = reinterpret_cast<ClientInitialHello*>(buf + sizeof(Header));

	if (headerReply->id == 1)
	{
		currentAssignedPlayer = clientHello->idPlayer;
		playerNames[currentAssignedPlayer] = name;
		//playerNames[currentAssignedPlayer] = clientHello->name;

		return true;
	}
	//current_players = atoi(buf);



	////HelloUpdate* helloUp = reinterpret_cast<HelloUpdate*>(buf);
	//////current_players = atoi(buf);
	////currentAssignedPlayer = helloUp->idPlayer;
	////playerNames[currentAssignedPlayer] = helloUp->name;

	//currentAssignedPlayer = atoi(buf);
	return false;
}


// client could receive hello update or Init game update from server
bool NetworkInterface::ClientLobbyReceiveUpdate(int& current_players, std::string* playerNames, std::vector<std::shared_ptr<House>>& houses, AI::AIWorld& world, std::vector<std::shared_ptr<Player>>& players, X::Color* playerColors, std::array<int, 8>& playerMonster)
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

	Header* headerReply = reinterpret_cast<Header*>(buf);


	if (bytes < 0)
	{
		//std::cerr << "Error receiveUpdate()\n";

		return false;
	}
	//eeee
	//else if (bytes < 8)
	else if (headerReply->id == 2)
	{
		current_players = headerReply->qty;

		auto playerSize{ sizeof(ClientInitialHello) };


		for (int i = 0; i < headerReply->qty; ++i)
		{
			//Player* b = reinterpret_cast<Player*>(buf + sizeof(Header) + (i * playerSize));
			ClientInitialHello* b = reinterpret_cast<ClientInitialHello*>(buf + sizeof(Header) + (i * playerSize));

			playerNames[b->idPlayer] = b->name;
			playerMonster[b->idPlayer] = b->idMonster;
			//playerNames[b->GetId()] = b->GetName();
			//b->
			//auto& house = houses.emplace_back(std::make_shared<House>(world));
			//house->position = b->circle.center;
			//house->radius = b->circle.radius;
			//house->SetRegenRate(b->regen);
			//house->Initialize();
			//house->SetUnits(b->units);
			//house->SetNetworkHouseId(b->networkId);
		}
		return false;
	}

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
				HouseInitialUpdate* b = reinterpret_cast<HouseInitialUpdate*>(buf + (i * bSize));

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

bool NetworkInterface::ClientLobbyUpdateMonsterType(int player, int monsterType)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ClientLobbyUpdateMonsterType()::pCommPort not initialized yet!\n";
		return false;
	}

	char buftest[64]{ 0 };

	Header header;
	header.id = 3;
	header.qty = 1;
	const auto headerSize{ sizeof(Header) };
	memcpy_s(buftest, headerSize, &header, headerSize);

	ClientInitialHello hello;
	hello.idPlayer = player;
	hello.idMonster = monsterType;
	//strcpy_s(hello.name, name);
	const auto clientInitialHelloSize{ sizeof(ClientInitialHello) };
	memcpy_s(buftest + headerSize, clientInitialHelloSize, &hello, clientInitialHelloSize);

	//memcpy_s(buftest + headerSize, sizeof(name), &name, sizeof(name));
	this->sendData(buftest, sizeof(buftest));
	//this->sendData(initialHello, std::strlen(initialHello));

	return true;
}

bool NetworkInterface::ServerGameSendHouseUpdate(std::vector<std::shared_ptr<House>>& houses)
{
	if (!this || !this->Initialized())
	{
		std::cerr << "ServerGameSendHouseUpdate()::pCommPort not initialized yet!\n";
		return true;
	}

	char buftest[512]{ 0 };

	Header header;
	header.id = 4;
	header.qty = houses.size();
	const auto headerSize{ sizeof(Header) };
	memcpy_s(buftest, headerSize, &header, headerSize);

	const auto houseSize{ sizeof(HouseUpdate) };

	std::vector<HouseUpdate> houseVector;
	for (auto& h : houses) {
		HouseUpdate tmp;
		tmp.idOwner = h->GetPlayerNetworkId();
		tmp.units = h->GetUnits();

		houseVector.push_back(tmp);
	}
	//this->sendDataBroadcast((char*)&houseVector[0], sizeof(HouseUpdate) * houseVector.size());

	memcpy_s(buftest + headerSize, houseSize * houseVector.size(), (char*)&houseVector[0], houseSize * houseVector.size());
	this->sendDataBroadcast(buftest, sizeof(buftest));

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

		const auto headerSize{ sizeof(Header) };
		const auto bUpdateSize{ sizeof(BUpdate) };
		
		int loops = ceil((bUpdateSize * static_cast<float>(size)) / 992.0f);
		int count_in_chunk_size = 992 / bUpdateSize;
		//assert(hCount * bUpdateSize == bytes);
		//assert(rCount == houses.size())

		const int test = scvs.size();
		std::vector<BUpdate> testin;
		
		testin.reserve(scvs.size());
		for (auto& scv : scvs) {
			auto& tmp = testin.emplace_back();
			tmp.pos = scv->position;
			//tmp.dest = scv->destination;
			//tmp.color = scv->GetOwner()->GetColor();
			tmp.ownerId = scv->GetOwner()->GetId();
		}
		if (loops == 1)
		{
			char buftest[1024]{ 0 };
			Header header;
			header.id = 5;
			//this->sendDataBroadcast((char*)&testin[0], sizeof(BUpdate) * size);
			header.qty = scvs.size();
			memcpy_s(buftest, headerSize, &header, headerSize);

			memcpy_s(buftest + headerSize, bUpdateSize * size, (char*)&testin[0], bUpdateSize * size);
			this->sendDataBroadcast(buftest, sizeof(buftest));
		}
		else
		{
			for (int i = 0; i < loops; ++i)
			{
				char buftest[1024]{ 0 };
				Header header;
				header.id = 5;
				//X::DrawScreenText(std::to_string(sizeof(BUpdate) * testscvs.size()).c_str(), 100.0f, 100.0f, 20.f, X::Colors::White);
				if (i == loops)
				{
					header.qty = size - (i * count_in_chunk_size);
					memcpy_s(buftest, headerSize, &header, headerSize);

					memcpy_s(buftest + headerSize, bUpdateSize * (size - (i * count_in_chunk_size)), (char*)&testin[i * count_in_chunk_size], bUpdateSize * (size - (i * count_in_chunk_size)));
					//this->sendDataBroadcast((char*)&testin[i * count_in_chunk_size], sizeof(BUpdate) * (size - (i * count_in_chunk_size)));
				}
				else
				{
					header.qty = count_in_chunk_size;
					memcpy_s(buftest, headerSize, &header, headerSize);

					memcpy_s(buftest + headerSize, bUpdateSize * count_in_chunk_size, (char*)&testin[i * count_in_chunk_size], bUpdateSize * count_in_chunk_size);
					//this->sendDataBroadcast((char*)&testin[i * count_in_chunk_size], sizeof(BUpdate) * count_in_chunk_size);
				}

				this->sendDataBroadcast(buftest, sizeof(buftest));
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

bool NetworkInterface::ClientGameReceiveHouseUpdate(std::vector<std::shared_ptr<House>>& houses, std::vector<std::shared_ptr<Player>>& players, std::array<X::TextureId, 4>& monsterTextures, std::array<int, 8>& playerMonster, std::array<X::Color, 9>& playerColors)
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
		Header* headerReply = reinterpret_cast<Header*>(buf);

		if (bytes < 0)
		{
			//std::cerr << "Error receiveUpdate()\n";
			return false;
		}
		else if (headerReply->id == 4)
		{
			auto hSize{ sizeof(HouseUpdate) };
			for (int i = 0; i < headerReply->qty; ++i)
			{
				HouseUpdate* b = reinterpret_cast<HouseUpdate*>(buf + sizeof(Header) + (i * hSize));

				houses[i]->SetUnits(b->units);
				houses[i]->SetPlayerOwner(b->idOwner);
				if ((b->idOwner) != 0)
					houses[i]->SetOwner(players[b->idOwner - 1]);
			}
		}
		else if (headerReply->id == 5)
		{

			const auto bSize{ sizeof(BUpdate) };
			const auto headerSize{ sizeof(Header) };
			//int rCount = bytes / bSize;
			//assert(rCount * bSize == bytes);
			//assert(rCount == BallCount);
			for (int i = 0; i < headerReply->qty; ++i)
			{
				BUpdate* b = reinterpret_cast<BUpdate*>(buf + headerSize + (i * bSize));
				//X::DrawSprite(textura, b->pos);
				if (b->pos.x >= 0 && b->pos.x <= 1280 && b->pos.y >= 0 && b->pos.y <= 720 && b->ownerId>0 && b->ownerId < 9) {
					X::DrawScreenCircle({ b->pos,20.0f }, playerColors[b->ownerId]);
					if (b->ownerId > 0 && b->ownerId < 5)
						X::DrawSprite(monsterTextures[playerMonster[b->ownerId]], b->pos);
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

void NetworkInterface::serializeLocalSample(char* buf)
{
	Header header;
	header.id = 1;
	header.qty = 1;

	const auto headerSize{ sizeof(Header) };


	//const auto objectSize{ sizeof(Player) };
	//for (int i = 0; i < players.size(); ++i)
	//{
	//	memcpy_s(buf + headerSize + (i * objectSize), objectSize, &players[i], objectSize);
	//}

}

