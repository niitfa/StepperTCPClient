#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <netinet/in.h>
#endif

#include "stepper_controller_reciever.h"
#include <cstring>
#include <vector>
#include <thread>
#include <iostream>

StepperControllerReciever::StepperControllerReciever(std::string ip, int port)
	: ip{ ip }, port{ port }
{
	memset(this->message, 0, kMessageSize);
}

int StepperControllerReciever::Connect()
{
	if (!this->started.load())
	{
		this->stopped.store(false);
		this->initialized.store(false);
		this->started.store(true);
		std::thread thrd(&StepperControllerReciever::ThreadHandler, this);
		thrd.detach();
		while (!this->initialized.load());
	}
	return 0;
}

int StepperControllerReciever::Disconnect()
{
	this->stopped.store(true);
	while (this->started.load());
	return 0;
}

uint32_t StepperControllerReciever::GetMessageID()
{
	this->mtx.lock();
	uint32_t val = *reinterpret_cast<uint32_t*>(this->message + this->kBytePosMessageID);
	this->mtx.unlock();
	return val;
}

int32_t StepperControllerReciever::GetLongEncoderValue()
{
	this->mtx.lock();
	int32_t val = *reinterpret_cast<int32_t*>(this->message + this->kBytePosLongEncValue);
	this->mtx.unlock();
	return val;
}

int32_t StepperControllerReciever::GetAngEncoderValue()
{
	this->mtx.lock();
	int32_t val = *reinterpret_cast<int32_t*>(this->message + this->kBytePosAngEncValue);
	this->mtx.unlock();
	return val;
}

void StepperControllerReciever::ThreadHandler()
{
#ifdef _WIN32
	/* ���������������� */
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		return;
	}

	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr(this->ip.c_str());
	addr.sin_port = htons(this->port);
	addr.sin_family = AF_INET;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);
#endif
#ifdef __linux__
	sockaddr_in hint;
	memset(&hint, 0, sizeof(hint));
	hint.sin_family = AF_INET;
	inet_pton(AF_INET, this->ip.c_str(), &hint.sin_addr.s_addr);
	//hint.sin_addr.s_addr = htonl(INADDR_ANY);
	hint.sin_port = this->port;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		std::cout << "failed to create client socket\n";
	}
	//socklen_t len = sizeof(hint);
#endif

	std::vector<char> message_vec;

	if (sock == -1)
	{
		return;
	}

	int bytes_received = 0;
	char buff[kBufferSize];
	memset(buff, 0, kBufferSize);

	this->initialized.store(true);
	while (!this->stopped.load())
	{
		int recv_size = recv(sock, buff, kBufferSize, 0);
		if (recv_size > 0)
		{
			bytes_received += recv_size;
			for (int i = 0; i < recv_size; ++i)
			{
				message_vec.push_back(buff[i]);
			}

			if (bytes_received == kMessageSize)
			{
				/* Make user copy of received message */
				/* Copy from vec to buff */
				for (int i = 0; i < message_vec.size(); ++i)
				{
					buff[i] = message_vec.at(i);
				}
				this->mtx.lock();
				memcpy(this->message, buff, kMessageSize);
				this->mtx.unlock();

				/* Clean buffer and vector */
				memset(buff, 0, kBufferSize);
				message_vec.clear();
				bytes_received = 0;
			}
		}
		else
		{
			memset(this->message, 0, kMessageSize);

#ifdef _WIN32
			closesocket(sock);
#endif
#ifdef __linux__
			close(sock);
#endif
			sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
			if (connect(sock, (SOCKADDR*)&addr, addrLen) < 0)
#endif
#ifdef __linux__
			if (connect(sock, (sockaddr*)&hint, sizeof(hint) < 0))
#endif
			{
				// Connection failed
				std::cout << "Connection failed\n";
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	memset(this->message, 0, kMessageSize);
#ifdef _WIN32
		closesocket(sock);
#endif

#ifdef __linux__
		close(sock);
#endif
	this->started.store(false);
}