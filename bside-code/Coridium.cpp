
/* C++ Specific */
#include <iostream>
#include <string>
#include <fstream>
#include <cerrno>

/* C Specific Includes */
extern "C" {
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
}

/* Our Own includes */
#include "Coridium.h"

using namespace std;

Coridium::Coridium()
{
	devicePort = "/dev/ttyUSB0";
}

/* Function use to set up the serial Port */
bool Coridium::stamp_init(void)
{
  /* Open Device
   *	Save old com port Settings
   *	set new com port Settings
   */
	device = fopen(devicePort.c_str(), "r+");

	if (device == NULL) {
	//	perror("ERROR Opening Device (Coridium): ");
		return 0;
	}

	/* File Control
	 Sets the file descriptor. Don't close after first execution */

	fcntl(fileno(device), F_SETFL, 0);

	/* save current COM port settings */
	tcgetattr(fileno(device),&oldSerialOptions);

	/* set new COM port settings */
	newSerialOptions.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
	newSerialOptions.c_cflag &= ~CRTSCTS;
	newSerialOptions.c_iflag = IGNPAR;
     newSerialOptions.c_lflag &= ~(ICANON); 
	newSerialOptions.c_lflag |= (ECHOE);

	/* Raw output */
	newSerialOptions.c_oflag |= OCRNL;
	newSerialOptions.c_oflag &= ~OPOST;

	newSerialOptions.c_line = 0;
	newSerialOptions.c_cc[VMIN] = 0;
	newSerialOptions.c_cc[VTIME] = 0;
	
	/* Enable Software Flow Control */
	newSerialOptions.c_iflag |= (IXON | IXOFF );

	/* Set speical Control Characters */
     newSerialOptions.c_cc[VSTART]   = 11;     /* (XON)*/ 
     newSerialOptions.c_cc[VSTOP]    = 13;     /* (XOFF)*/

	/* Push new settings to the Device */
	tcsetattr(fileno(device),TCSANOW,&newSerialOptions);

	return 1;
}

/* Reset the stamp */
void Coridium::stamp_reset(void)
{
	int dtr_flag;

	/* 1. set DTR high */
	dtr_flag = TIOCM_DTR;
	ioctl(fileno(device), TIOCMBIS, &dtr_flag);

	/*  Set break condition on TX */
	ioctl(fileno(device), TIOCSBRK, 1);
	
	/* 3. Sleep for 100 Miliseconds */
	usleep(100);

	/* 4. Set DTR low: */
	dtr_flag = TIOCM_DTR;
	ioctl(fileno(device), TIOCMBIC, &dtr_flag);
	
	/* 5. Clear break condition on TX */
	ioctl(fileno(device), TIOCCBRK, 1);

	/* ... and flush receive buffer: */
	tcflush(fileno(device),TCIFLUSH);

}

/* Identify the stamp */
bool Coridium::stamp_id(void)
{
	char tempChar;
	char ver[3];

	int read_sz = 0;
	int i;

	stamp_reset();

	do{
		if (fread(&tempChar, 1, 1, device)) {
			if (tempChar != char(0) && tempChar != char(10)) {
				recvBuffer[read_sz] = tempChar;
				read_sz++;
			}
		}
	}while(read_sz < 15);

	if (recvBuffer[0] != char(65)){
		stamp_halt();
		return 0;
	}
	
	model = "ARMexpress";

	for ( i = 0; i < 4; i++){
		ver[i] = recvBuffer[9+i]; 
	}

	firmwareVer = atof(ver);

	return 1;
}

void Coridium::stamp_halt(void)
{
	/* Check to make sure the device is open before we close it */
	if( device != NULL) {
		/* restore original port settings and close it */
		tcsetattr( fileno(device),TCSANOW,&oldSerialOptions);
		fclose(device);
	}

}

string Coridium::get_model(void)
{
	return model;
}

float Coridium::get_firmware(void)
{
	return firmwareVer;
}

void Coridium::complete_sequence(void)
{

}

char Coridium::stamp_send_bytes(char snd)
{
	/* Send the identification request character */
	if (fwrite(&snd,1,1,device) > 0){
		/* Receive and discard echo: */
		stamp_read(recvBuffer, 1);
		stamp_read(recvBuffer, 1);
	}
	
	return recvBuffer[0];
}

bool Coridium::stamp_sendfile(string myFile)
{
	int write_sz, read_sz;
	char line;
	unsigned char outline;
	ifstream inFile;

	inFile.open(myFile.c_str(), ios::in);

	if ( !inFile.is_open() ) {
		perror("ERROR");
		return(0);
	}
	
	stamp_reset();
	stamp_sendCmd(1);

	while ( inFile.readsome(&line, 1) > 0) {
		read_sz = inFile.gcount();
		cout << "Read in " << read_sz << " bytes: " << line << endl;
		write_sz = write(fd, &line, read_sz);
		cout << "Wrote to Stamp " << write_sz << " byte(s)" << endl; 
		read(fd, &outline, write_sz);
		printf("Stamp echoed back %d  bytes: %d\n", write_sz, outline);
		tcflush(fd,TCIFLUSH);		
	}

	stamp_sendCmd(2);
	tcflush(fd,TCIFLUSH);		
	
	inFile.close();

	complete_sequence();

	return 1;
}


void Coridium::stamp_sendCmd(int num) {

	char tempChar;

	switch(num) {

		case 1:
				/* Send Clear: Used to Clear the Stamp */
				write(fd, "CLEAR",5);	
				break;
		case 2:
				/* Send Run: Used to Run Whatever is on the stamp */
				write(fd, "RUN",3);
				break;
		case 3:
				/* Send Esc: Used to Enter Programming Mode */
				tempChar = char(27);
				write(fd, &tempChar, 1);
				break;
		case 4:
				/* Send Ctrl-C: Used to Enter Programming Mode */
				tempChar = char(3);
				write(fd, &tempChar, 1);
				break;
		default:
				break;
	}
}

void Coridium::stamp_read(char *str, int count)
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

int Coridium::get_fd()
{
	return fileno(device);
}


string Coridium::get_devicePort(void)
{
	return devicePort;
}

void Coridium::set_devicePort(string s)
{
	devicePort = s;
}
