#pragma once


// platform detection
#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>
#include <WS2tcpip.h>


#pragma comment(lib, "Ws2_32.lib")

#elif PLATFORM == PLATFORM_MAC || 
PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

struct Address
{
public:
	Address(unsigned char a,
		unsigned char b,
		unsigned char c,
		unsigned char d,
		unsigned short port);

	Address(char* address, unsigned short port);

	Address() = default;


	sockaddr_in* GetSockAddress()
	{
		return &(m_sockaddress);
	}

	unsigned short GetPort() const
	{
		return m_port;
	}

	void Set(char* address, unsigned short port);
	void Set(unsigned int add, unsigned short p);
	bool IsServer() const;
	void SetPort(unsigned short p) { m_port = p; }


	unsigned short m_port;
	sockaddr_in m_sockaddress;
	bool m_isServer;
};

