#ifndef _PARALLAX_H
#define _PARALLAX_H

#include <termios.h>

class Parallax
{
	public: 
		Parallax();
		~Parallax(){};
		void set_firmware(char ackChar);
		void set_model(char);	
		void complete_sequence(void);
		bool stamp_init(void);
		void stamp_reset(void);
		bool stamp_id(void);
		char stamp_send_bytes(char);
		bool stamp_sendfile(std::string, int);
		void error_handler(char);
		void set_devicePort(std::string);
		std::string get_devicePort(void);
		float get_firmware(void);
		void stamp_halt(void);
		void stamp_read(char *, int);
		std::string get_model(void);
		int get_fd(void);
	
	protected:
		int programSlot;	/* Slot to store Program */
		int slots;		/* Number of slots */
		std::string model;	/* Model of the stamp */
		float firmwareVer;	/* Firmware of the stamp */
		FILE *device;
		std::string devicePort;	
		char sendBuffer[1024];
		char recvBuffer[1024];
		
		struct termios newSerialOptions;
		struct termios oldSerialOptions;
	
};
#endif
