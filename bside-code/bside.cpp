/*	Project Information:
 *		Name: BSide(Basic Stamp IDE)
 *		Page: http://www.devscott.org/projects/bside
 *		License: GPL
 * 		
 * 		About:
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1		/* enables some compiler checks on GNU */
#endif

#define DEBUG 1

#include <iostream>
#include <string>
#include <fstream>
#include <cerrno>
#include <iomanip>

extern "C" {
#include <signal.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
}

#include "stamp.h"

using namespace std;

stamp myStamp;

/*General Stuff */

float bside_version = 0.04;		/* Version of Software */
string myFile = "";	    		/* File to be transfered */
string device;        			/* Stamp device */

/* Stamp Stuff */

int programSlot = 0;			/* Program Slot for Parallax stamps */
int fd;

/* Debug Stuff */
bool debug_loop_done;
bool testRun;

void alarmTimeout(int);
void *control_c_handler(int sig);
void stamp_debug(void);

static void help()
{
	printf("Usage: bside -d [device] -f [filename]\n\n");
	printf("Arguments:\n\n");
	printf("\t-d    --device\t Specify the desired device path\n");
	printf("\t-f    --file\t Specify file to send to the Stamp\n");
	printf("\t-h    --help\t This screen\n");						
	printf("\t-s    --slot\t Set the Program Slot (0-7) Parallax Stamps Only\n");
	printf("\t-t    --test\t Run in Test Mode\n");						
	printf("\t-v    --version\t Get version information\n");
	printf("\n");
}

static void version()
{
	printf("BSide Version: %f\n", bside_version);
}

int main(int argc, char **argv)
{	
	bool mode_test = false;
	int opt; 

	static struct option long_options[] = {
		{ "device", 	required_argument, 	0,	'd'},
		{ "file", 	required_argument, 	0,	'f'},
		{ "help", 	no_argument,		NULL, 	'h'},
		{ "slot", 	required_argument, 	0,	'p'},
		{ "test",	no_argument,		0,	't'},
		{ "version", 	no_argument, 		NULL, 	'v'},
		{0, 0, 0, 0}
	};


	if ( argc > 1)
	{
		while ((opt = getopt_long(argc, argv, ":th?s:vd:f:", long_options, NULL) )!= -1)
		{
			switch(opt)
			{
				case 'h': case '?':
					help();
					return 0;
					break;
				case 'd':
					myStamp.set_devicePort(strdup(optarg));     
					#ifdef DEBUG    
						cout << "Set to use device: " << optarg << endl;    
					#endif      
					break;
				case 'f':
					myFile = optarg;
					#ifdef DEBUG
						cout << "Set to send file: " << myFile << endl;
					#endif	
					break;
				case 's':
					programSlot = atoi(optarg);
					#ifdef DEBUG
						cout << "Program will reside at slot: " << programSlot << endl;
					#endif
					break;				
				case 't':
					mode_test = true; 
					break;		
				case 'v':
					version();
					return 0;
					break; 
				default:
					printf("Usage: bside -d [device] -f [filename]\n\n");
					break;
			}
		}
	}	
	else
		testRun = 1;

	/* set Alarm for 20 seconds */
	signal(SIGALRM,alarmTimeout);
	alarm(5);

	/* Setup the Stamp and try to identify it*/
	if (!myStamp.setup()){

		cout <<"Stamp could not be found or identified!\n";
		cout <<"If no reason is stated, Possible failure are:\n";
		cout <<"\t -Unsupported Stamp or firmware\n";
		cout <<"\t -Dead Battery on Board\n";
		cout <<"\t -Didn't specify proper device\n";
		return 1;
	}
	
	/* Disable the Alarm */
	alarm(0);

	cout  << "\nBasic Stamp detected!" << endl;
	cout  << "Model: "<< myStamp.get_model() << endl;
	cout  << "Vender: " << myStamp.get_vender() << endl;
	cout  << "Firmware version: " << setprecision(2) <<  myStamp.get_firmware() << endl;		

	if (!mode_test){
		/* If inFile was set, send it to the stamp */
		if( myFile.compare("") != 0){

			if (!myStamp.send_file(myFile, programSlot) ){

				cerr << "Transfering File Failed! (Reason Stated Above)\n";
				myStamp.halt();
				return 1;
			}

			cout << myFile << " transferred successfully!\n";
		}
	}
		
	/* Show the Debug Screen */
	stamp_debug();

	/* Kill everything */
	myStamp.halt();
	
	return 0;
}

/* Debug output  */
void stamp_debug(void)
{
	fd = myStamp.get_fd();
	char recvchar;

	myStamp.reset();

	cout << "Entering Debug Mode:\n";
	cout << "DEBUG OUTPUT:     (Press [Ctrl] + [c] to complete sequence)\n";
	cout << "_______________________________________________________________\n";

	signal(SIGINT,(sighandler_t)control_c_handler);
	debug_loop_done = 0;

	do
	{
    		fflush(stdout);

		if (read(fd,&recvchar, 1))
	  	{
	    		cout << recvchar;

	    		/* Take into account DOS EOL: */
	    		if (recvchar == '\r')
	      			cout << endl;

	    		fflush(stdout);
	  	}
      		
	}while (!debug_loop_done);
}

/* handle control c */
void *control_c_handler(int sig)
{
	fflush(stdout);
	fflush(stderr);
	
	cout << "_______________________________________________________________\n";
	cout << "\nUser Aborted\n";

	debug_loop_done = 1;
	return NULL;
}

/* Handle any timeout */
void alarmTimeout(int signal)
{
	cout << "Could not detect a stamp. Please check Your connections\n";
	exit(-13);
}
