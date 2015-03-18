
/* C++ Specific */
#include <iostream>
#include <string>
#include <fstream>
#include <cerrno>

/* C Specific Includes */
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/* Our Own includes */
#include "BasicX.h"

using namespace std;

BasicX::BasicX()
{
	devicePort = "/dev/ttyS0";
	firmwareVer = 0;
	model = "";
}

/* Function use to set up the serial Port */
bool BasicX::stamp_init(void)
{

}

/* Reset the stamp */
void BasicX::stamp_reset(void)
{

}

/* Identify the stamp */
bool BasicX::stamp_id(void)
{

	return 1;
}

void BasicX::stamp_halt(void)
{
	/* restore original port settings and close it */
	tcsetattr( fileno(device),TCSANOW,&oldSerialOptions);

	if (fclose(device) )
		perror("ERROR: Halting BasicX Stamp");
		
}

/* set the Firmware version based on Ack char */
void BasicX::set_firmware(char id)
{

}

/* set the Model Name based on Ack char */
void BasicX::set_model(char id)
{

}

void BasicX::set_firmware(float fw)
{

}

void BasicX::complete_sequence(void)
{

}



char BasicX::stamp_send_bytes(char snd)
{

}

bool BasicX::stamp_sendfile(string myFile)
{
	int write_sz, read_sz;
	char buffer[18];
	char outbuffer[18];
	char ackChar;
	ifstream inFile;

	inFile.open(myFile.c_str(), ios::in);

	if ( !inFile.is_open() ){
		perror("ERROR");
		return 0;
	}	

	
	inFile.close();

	return 1;
}

void BasicX::error_handler(char recv)
{

}

void BasicX::stamp_read(char *str, int count)
{
	int i, recv = 0;
	char chr;

	for (i = 0; i < count; i++){
		
		if (fread( &chr, 1, 1, device) <= 0 ){
			recvBuffer[i] = char(0);
		} else {
			recvBuffer[i] = chr;
			recv++;
		}
	}
}

string BasicX::get_model(void)
{
	return model;
}

float BasicX::get_firmware(void)
{
	return firmwareVer;
}

void BasicX::set_devicePort(string s)
{
	devicePort = s;
}

string BasicX::get_devicePort(void)
{
	return devicePort;
}

int BasicX::get_fd()
{
	return fileno(device);
}
