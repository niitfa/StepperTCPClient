#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#ifdef _linux
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
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
	/* инициализируемся */
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

	std::vector<char> message_vec;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);

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
			closesocket(sock);
			sock = socket(AF_INET, SOCK_STREAM, NULL);
			if (connect(sock, (SOCKADDR*)&addr, addrLen) < 0)
			{
				// Connection failed
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	memset(this->message, 0, kMessageSize);
	closesocket(sock);
	this->started.store(false);
#endif

#ifdef _linux
#endif
}