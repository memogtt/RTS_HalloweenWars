#pragma once
#include "Address.h"
#include <vector>


class CSocket
{
public:
	CSocket() = delete;
	CSocket(char* address, unsigned short port);

	~CSocket();

	bool Open();

	void Close();

	bool IsOpen() const;

	bool isServer() const;

	bool Send(Address& destination,
		const void* data,
		int size);

	int Receive(Address& sender,
		void* data,
		int size);

private:

	int m_handle;
	Address m_address;
	std::vector<Address> mAddressVector;
};
