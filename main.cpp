#include <iostream>
#include <thread>
#include <vector>

#include "stepper_controller_reciever.h"

int main()
{
	StepperControllerReciever* receiver = new StepperControllerReciever("127.0.0.1", 6101);

	receiver->Connect();

	while (true)
	{
		int id = receiver->GetMessageID();
		int long_pos = receiver->GetLongEncoderValue();
		int ang_pos = receiver->GetAngEncoderValue();

		std::cout
			<< "Msg ID: " << id
			<< "\tLong pos: " << long_pos
			<< "\tAng pos: " << ang_pos
			<< std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	receiver->Disconnect();

	return 0;
}