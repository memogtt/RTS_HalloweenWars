#include <iostream>
#include <assert.h>
#include "Address.h"
#include "NetworkInterface.h"


#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#endif


struct PlayerN {
	int id = 0;
	Address clientAddress;

	//virtual ~Player() {};
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
