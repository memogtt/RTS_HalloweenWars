
#include <iostream>
#include "Address.h"


Address::Address(
	unsigned char a,
	unsigned char b,
	unsigned char c,
	unsigned char d,
	unsigned short port) : m_port(port), m_isServer(false)
{
	int address = (a << 24) | (b << 16) | (c << 8) | d;
	memset((char*)&m_sockaddress, 0, sizeof(m_sockaddress));
	m_sockaddress.sin_family = AF_INET;
	m_sockaddress.sin_addr.s_addr = address;
}

Address::Address(char* address, unsigned short port) :
	m_port(port), m_isServer(false)
{
	Set(address, port);
}

void Address::Set(char* address, unsigned short port)
{
	m_port = port;
	memset((char*)&m_sockaddress, 0, sizeof(m_sockaddress));
	m_sockaddress.sin_family = AF_INET;
	m_sockaddress.sin_port = htons((unsigned short)port);
	if (strcmp(address, "0") != 0)// this is a client
	{

		inet_pton(AF_INET, (PCSTR)address, &m_sockaddress.sin_addr.s_addr);	
		//m_sockaddress.sin_addr.s_addr = inet_addr(address);
		m_isServer = false;
		std::cout << "THis is a client\n";
	}
	else // this is the server
	{
		m_sockaddress.sin_addr.s_addr = INADDR_ANY; // this for server side
		std::cout << "This is the server\n";
		m_isServer = true;
	}

}

void Address::Set(unsigned int add, unsigned short p)
{
	m_port = p;
	memset((char*)&m_sockaddress, 0, sizeof(m_sockaddress));
	m_sockaddress.sin_family = AF_INET;
	m_sockaddress.sin_addr.s_addr = add;
	m_sockaddress.sin_port = htons((unsigned short)p);
}

bool Address::IsServer() const
{
	return m_isServer;
}
