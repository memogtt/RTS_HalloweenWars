#pragma once

#include "CSocket.h"
#include <vector>

struct PlayerN;

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
	int recvData(char* buf, int sz, int &playerOut);
	bool isServer() const;
	void AddPlayer(unsigned char a,
		unsigned char b,
		unsigned char c,
		unsigned char d)
	{}

	CSocket m_socket;
	bool Initialized() const;
	std::vector<PlayerN> mPlayers;
	//std::vector<std::unique_ptr<Player>> mPlayers;
	int currentPlayers;

};