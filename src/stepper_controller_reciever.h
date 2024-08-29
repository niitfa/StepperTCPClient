#ifndef STEPPER_CONTROLLER_RECIEVER_H
#define STEPPER_CONTROLLER_RECIEVER_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class StepperControllerReciever
{
	/* Message info */
	static const int kMessageSize = 12;
	static const int kBufferSize = kMessageSize;
	char message[kMessageSize];

	static const int kBytePosMessageID		= 0;
	static const int kBytePosLongEncValue	= 4;
	static const int kBytePosAngEncValue	= 8;

	/* Server info */
	std::string ip;
	int port;

	/* Thread info */
	std::mutex mtx;
	std::atomic<bool> started		{ false };
	std::atomic<bool> initialized	{ false };
	std::atomic<bool> stopped		{ false };
public:
	/**
	* @brief Constructor
	* @param[in] ip Server IP
	* @param[in] port Server port
	*/
	StepperControllerReciever(
		std::string ip,
		int port
	);
	/**
	* @brief Destructor
	*/
	~StepperControllerReciever() = default;

	/**
	* @brief Creates TCP socket and connects to server.
	*/
	int Connect();

	/**
	* @brief Closes TCP socket.
	*/
	int Disconnect();

	/**
	* @brief Returns ID of last received message
	*/
	uint32_t GetMessageID();

	/**
	* @brief Returns longitudal axis encoder value
	*/
	int32_t GetLongEncoderValue();

	/**
	* @brief Returns angular axis encoder value
	*/
	int32_t GetAngEncoderValue();
private:
	void ThreadHandler();
};

#endif

