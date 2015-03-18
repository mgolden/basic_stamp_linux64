
/* C++ Specific */
#include <string>
#include <fstream>
#include <cerrno>

/* C Specific Includes */
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/* Our Own includes */
#include "Parallax.h"

using namespace std;

Parallax::Parallax()
{
	devicePort = "/dev/ttyS0";
	firmwareVer = 0;
	model = "";
}

/* Function use to set up the serial Port */
bool Parallax::stamp_init(void)
{
  /* Open Device
   *	Save old com port Settings
   *	set new com port Settings
   */
	device = fopen(devicePort.c_str(), "r+");

	if (device == NULL) {
	//	perror("ERROR Opening Device (Parallax): ");
		return 0;
	}

	/* File Control
	 Sets the file descriptor. Don't close after first execution */

	fcntl(fileno(device), F_SETFL, 0);

	/* save current COM port settings */
	tcgetattr(fileno(device),&oldSerialOptions);

	/* set new COM port settings */
	newSerialOptions.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	newSerialOptions.c_iflag = IGNPAR;
	newSerialOptions.c_oflag = 0;
	newSerialOptions.c_lflag = 0;
	newSerialOptions.c_line = 0;
	newSerialOptions.c_cc[VMIN] = 0;
	newSerialOptions.c_cc[VTIME] = 5;
	
	/* Push new settings to the Device */
	tcsetattr(fileno(device),TCSANOW,&newSerialOptions);

	return 1;
}

/* Reset the stamp */
/*
 *	From BASIC Stamp Programming Protocol: p.5
 * 
 *	DTR & Tx High for 2ms,
 *	DTR Low & Tx High for 36 ms,
 *	DTR & Tx Low for 20 ms,
 *	Flush buffer
 */
void Parallax::stamp_reset(void)
{
	int dtr_flag;

	/* 1. set DTR high */
	dtr_flag = TIOCM_DTR;
	ioctl(fileno(device), TIOCMBIS, &dtr_flag);

	/* 2. Set break condition on TX */
	ioctl(fileno(device), TIOCSBRK, 4);

	/* 3. Pause for at least 2 ms: */
	usleep(2000);

	/* 4. Set DTR low: */
	dtr_flag = TIOCM_DTR;
	ioctl(fileno(device), TIOCMBIC, &dtr_flag);

	/* 5. Pause for at least 36 ms: */
	usleep(36000);

	/* 6. Clear break condition on TX */
	ioctl(fileno(device), TIOCCBRK, 4);

	/* 7. Pause for approximately 20 ms */
	usleep(20000);

	/* ... and flush receive buffer: */
	tcflush(fileno(device),TCIFLUSH);

}

/* Identify the stamp
 * Steps: 
 *	reset the stamp,
 *	try the various stamp recognition sequences,
 *	set the type and version,
 *	return 1 if good, 0 otherwise
 */

bool Parallax::stamp_id(void)
{
	char snd[] = {'e','X','P','I'};
	int count = -1;	
	char tempChar;

	stamp_reset();

	/* check for BS2 */
	if (stamp_send_bytes('B') == char(190))
	{
		if (stamp_send_bytes('S') == char(173))
		{
			if (stamp_send_bytes('2') == char(206)){
				tempChar = stamp_send_bytes(0);
				set_firmware(tempChar);			
				set_model('B');
			}
		} else {
			stamp_halt();
			return 0;
		}
	}

	if ( model.compare("") == 0 ){
		do{
			count++;			
			stamp_reset();

			tempChar = stamp_send_bytes(snd[count]);

			set_firmware(tempChar);
			set_model(tempChar);
		
		}while(count < 3);
	}

	if (model.compare("") == 0 ){
		stamp_halt();
		return 0;
	}
	return 1;
}

void Parallax::stamp_halt(void)
{
	/* Check to make sure the device is open before we close it */
	if( device != NULL) {
		/* restore original port settings and close it */
		tcsetattr( fileno(device),TCSANOW,&oldSerialOptions);
		fclose(device);
	}
}

/* set the Firmware version based on Ack char */
void Parallax::set_firmware(char id)
{
	int charValue = (int)id;
	int temp;

	switch (id)
	{
		case 'e': case 'i': case 'I': case 'P': case 'p': case 'X': 
			firmwareVer = 1.0;
			break;
		case 'j': case 'J': case 'q': case 'Q': case 'Y': 
			firmwareVer = 1.1;
			break;
		case 'k': case 'K': case 'r': case 'R': case 'Z':
			firmwareVer = 1.2;
			break;
		case 's': case 'S':
			firmwareVer = 1.3;
			break;
		default:		/*BS2 sends the firm version in BCD Form. We get the value here */ 
			while ( charValue > 0 ) 
			{
				if ( charValue / 16 > 0 )
				{
					temp = (charValue /16) ;  
					firmwareVer = static_cast<float>(temp);
					temp *= 16;
				}
				else
				{
					temp = charValue %16;
					
					if ( temp >= 10 )
						firmwareVer += static_cast<float>(temp) / 100;
					else 
						firmwareVer += static_cast<float>(temp) / 10;
				}
				
				charValue -= temp;
			}
			break;
		
	}
}

string Parallax::get_model(void)
{
	return model;
}

float Parallax::get_firmware(void)
{
	return firmwareVer;
}

/* set the Model Name based on Ack char */
void Parallax::set_model(char id)
{
   	switch (id)
	{	
		case 'B':
			model= "BS2";
			break;
   		case 'X': case 'Y': case 'Z':
      			model = "BS2sx";
      			break;
   		case 'e':
      			model = "BS2e";
      			break;
   		case 'p': case 'q': case 'r': case 's':
      			model = "BS2p24";
      			break;
		
   		case 'P': case 'Q': case 'R': case 'S':
      			model = "BS2p40";
      			break;
		
   		case 'i': case 'j': case 'k':
      			model = "BS2pe24";
      			break;
		
   		case 'I': case 'J': case 'K':
      			model = "BS2pe40";
      			break;
  	}

	if (model.compare("BS2") == 0 )
		slots = 0;
	else
		slots = 7;
}

void Parallax::complete_sequence(void)
{
	/* Send Complete Sequence */
	fwrite("0", 1, 1, device);
}

void Parallax::set_devicePort(string s)
{
	devicePort = s;
}

string Parallax::get_devicePort(void)
{
	return devicePort;
}

char Parallax::stamp_send_bytes(char snd)
{
	/* Send the identification request character */
	if (fwrite(&snd,1,1,device) > 0){
		/* Receive and discard echo: */
		stamp_read(recvBuffer, 1);
		stamp_read(recvBuffer, 1);
	}
	
	return recvBuffer[0];
}

bool Parallax::stamp_sendfile(string myFile, int programSlot)
{
	int write_sz, read_sz;
	char buffer[18];
	char outbuffer[18];
	char ackChar;
	ifstream inFile;

	inFile.open(myFile.c_str(), ios::in);

	if ( !inFile.is_open() ) {
		perror("ERROR");
		return 0;
	}	

	if ( model.compare("BS2") != 0 ){

		if ( programSlot < 0 || programSlot > slots){
			throw("Invalid Slot: Using Slot 0");
			programSlot = 0 ;
		}

		char slot = (char)programSlot;
		fwrite(&slot, 1, 1, device);
	}

	while( inFile.readsome(buffer, 18) > 0)
	{
		read_sz = inFile.gcount();
			
		write_sz = write(fileno(device),buffer, read_sz);
			
		tcflush(fileno(device),TCIFLUSH);
		usleep(500000);
		
		/* Read and discard 18-byte packet */
		read_sz = read(fileno(device), outbuffer, read_sz);	
		/* Get ack char */
		read(fileno(device),&ackChar, 1);
			
		if( ackChar != 0)
		{
			error_handler(ackChar);
			inFile.close();			
			return 0;
		}
	}
	
	inFile.close();

	complete_sequence();

	return 1;
}

void Parallax::error_handler(char recv)
{
	switch(recv)
	{
		case '1':
			throw("Error: Communication error (checksum didn't match received data) try again");
			break;
		case '2':
			throw("Error: EEPROM error (at least one EEPROM location failed to retain data)");
			break;
		default:
			throw("Unknown error code");
			break;
	}
}

void Parallax::stamp_read(char *str, int count)
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

int Parallax::get_fd()
{
	return fileno(device);
}
