#include <iostream>
#include "CSocket.h"
#include "Address.h"


bool InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
	return true;
#endif
}

void ShutdownSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

CSocket::CSocket(char* address, unsigned short port) :
	m_handle(-1), m_address(address, port)
{
	if (!InitializeSockets())
	{
		std::cout << "Failed to initialize winsocket!2\n";
		return;
	}
}

CSocket::~CSocket()
{
	ShutdownSockets();
}

bool CSocket::Open()
{
	m_handle = INVALID_SOCKET;
	auto handle = m_handle;
	if (m_address.IsServer()) // this is for server
	{
		handle = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else // this is for client
	{
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	if (handle == INVALID_SOCKET)
	{
		std::cout << "failed to create socket\n";
		return false;
	}

	// make socket non-blocking
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

	int nonBlocking = 1;
	if (fcntl(m_handle,
		F_SETFL,
		O_NONBLOCK,
		nonBlocking) == -1)
	{
		std::cout << ("failed to set non-blocking\n");
		return false;
	}
#elif PLATFORM == PLATFORM_WINDOWS

	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0)
	{
		std::cout << ("failed to set non-blocking\n");
		return false;
	}
#endif

	m_handle = handle;

	// using it in interactive programs, meaning that do not delay sending individual messages.
	const char OPTION_VALUE = 1;
	setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, &OPTION_VALUE, sizeof(int));

	if (m_address.IsServer()) // this is for server
	{
		if (handle != INVALID_SOCKET)
		{
			if (bind(handle, (const sockaddr*)(m_address.GetSockAddress()), sizeof(sockaddr_in)) == SOCKET_ERROR)
			{
				std::cout << ("failed to bind socket\n");
				Close();
				return false;
			}
		}
	}
	return true;
}

bool CSocket::IsOpen() const
{
	return m_handle > 0;
}

bool CSocket::isServer() const
{
	return m_address.IsServer();
}

void CSocket::Close()
{
#if PLATFORM == PLATFORM_WINDOWS
	closesocket(m_handle);
#else
	close(m_handle);
#endif
	m_handle = INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////
bool CSocket::Send(Address& destination,
	const void* data,
	int size)
{
	int sent_bytes{ 0 };

	if (isServer()) {
		sent_bytes =
			sendto(m_handle,
				(const char*)data,
				size,
				0,
				(struct sockaddr*)destination.GetSockAddress(),
				sizeof(sockaddr));
	}
	else
	{
		sent_bytes =
			sendto(m_handle, //destination.GetPort(),
				(const char*)data,
				size,
				0,
				(struct sockaddr*)m_address.GetSockAddress(),
				sizeof(sockaddr));
	}
	if (sent_bytes == SOCKET_ERROR)
	{
		std::cerr << "Failed sending message: " << WSAGetLastError() << "\n";
		return false;
	}
	if (sent_bytes != size)
	{
		std::cerr << "failed to send the full packet: " <<
			sent_bytes << ":" << size << " error:" << WSAGetLastError() << "\n";
		return false;
	}
	return true;
}

int CSocket::Receive(Address& sender, void* data, int size)
{
	sockaddr_in from{};
	int fromLength = sizeof(from);

	int bytes = recvfrom(m_handle,
		(char*)data,
		size,
		0,
		(sockaddr*)&from,
		&fromLength);

	if (bytes <= 0)
	{
		std::cerr << "No data was received: " << WSAGetLastError() << "\n";
		return bytes;
	}

	if (bytes == 0)
	{
		std::cerr << "Receive 0: " << WSAGetLastError() << "\n";
		return bytes;
	}

	//unsigned int from_address = ntohl(from.sin_addr.s_addr);

	sender.m_sockaddress = from;
	if (isServer()) {
		m_address.m_sockaddress = from;

		//mAddressVector.push_back(mfrom)

	}
	return bytes;
}
