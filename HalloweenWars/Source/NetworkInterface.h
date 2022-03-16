#pragma once

#include "CSocket.h"
#include <vector>

#include <AI.h>
#include <XEngine.h>

struct PlayerN;

class Player;
class House;
class SCV;

class NetworkInterface
{
public:
	NetworkInterface(char* ip, unsigned short port);
	~NetworkInterface();

	//void SetUpSocket();
	bool sendData(char* msg, int size);
	bool sendDataTo(char* msg, int size, int playerNumber);
	bool sendDataBroadcast(const char* msg, int size);
	int recvData(char* buf, int sz);
	int recvData(char* buf, int sz, int& playerOut);
	bool isServer() const;
	void AddPlayer(unsigned char a,
		unsigned char b,
		unsigned char c,
		unsigned char d)
	{}

	bool Initialized() const;

	bool ServerLobbyWaitHello(std::vector<std::shared_ptr<Player>>& players, int& current_players, std::string* playerNames, X::Color* playerColors, std::array<int, 8>& playerMonster);
	bool ClientLobbySendHello(char* name, int& currentAssignedPlayer, std::string* playerNames);
	bool ClientLobbyReceiveUpdate(int& current_players, std::string* playerNames, std::vector<std::shared_ptr<House>>& houses, AI::AIWorld& world, std::vector<std::shared_ptr<Player>>& players, X::Color* playerColors, std::array<int, 8>& playerMonster);
	bool ServerLobbySendInitGame(std::vector<std::shared_ptr<House>>& houses);
	bool ClientLobbyUpdateMonsterType(int player, int monsterType);

	bool ServerGameSendHouseUpdate(std::vector<std::shared_ptr<House>>& houses);
	bool ServerGameSendMonsterUpdate(std::vector<std::unique_ptr<SCV>>& scvs);
	bool ServerGameReceiveCommand(std::vector<std::shared_ptr<House>>& houses, std::vector<std::unique_ptr<SCV>>& scvs, std::vector<std::shared_ptr<Player>>& players, AI::AIWorld& world);
	bool ClientGameReceiveHouseUpdate(std::vector<std::shared_ptr<House>>& houses, std::vector<std::shared_ptr<Player>>& players, std::array<X::TextureId, 12>& monsterTextures, std::array<int, 8>& playerMonster, std::array<X::Color, 9>& playerColors);
	bool ClientGameSendCommand(int idPlayer, int idHouseOrigin, int idHouseDestination, int percentage);

	void serializeLocalSample(char* buf);


	CSocket m_socket;
	std::vector<PlayerN> mPlayers;
	//std::vector<std::unique_ptr<Player>> mPlayers;
	int currentPlayers;
	int animLoop = 1;

};