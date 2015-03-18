#ifndef _CORIDIUM_H
#define _CORIDIUM_H

#include <termios.h>

class Coridium
{
	public: 
		Coridium();
		~Coridium(){};
		void complete_sequence(void);
		bool stamp_init(void);
		void stamp_reset(void);
		bool stamp_id(void);
		char stamp_send_bytes(char);
		bool stamp_sendfile(std::string);
		void set_devicePort(std::string);
		std::string get_devicePort(void);
		float get_firmware(void);
		void stamp_halt(void);
		void stamp_read(char *, int);
		std::string get_model(void);
		int get_fd(void);
		void stamp_sendCmd(int);

	protected:
		std::string model;	/* Model of the stamp */
		float firmwareVer;	/* Firmware of the stamp */
		FILE *device;
		std::string devicePort;	
		char sendBuffer[1024];
		char recvBuffer[1024];
		int fd;
		
		struct termios newSerialOptions;
		struct termios oldSerialOptions;
	
};
#endif
