/* Basic Stamp communication */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

enum {
  MODEL_BS2,
  MODEL_BS2e,
  MODEL_BS2sx,
  MODEL_BS2p24,
  MODEL_BS2p40,
  MODEL_BS2pe24,
  MODEL_BS2pe40,
  MODEL_UNKNOWN
};

const char * model_names[MODEL_UNKNOWN + 1] = {
  "Basic Stamp 2",
  "Basic Stamp 2e",
  "Basic Stamp 2sx",
  "Basic Stamp 2p24",
  "Basic Stamp 2p40",
  "Basic Stamp 2pe24",
  "Basic Stamp 2pe40",
  "UNKNOWN"
};

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

/*#define DEBUG          1*/

//#define TRUE           1
#define FALSE          0
#define STATE_CRITICAL -13

//#define OK             0
//#define ERROR          -1

#define DEFAULT_TIMEOUT 10

#define BUFFER_SIZE    1024

extern int errno;

//int show_usage=FALSE;

const char *device="/dev/ttyUSB0";
//int probe_number=1;
//int battery_ok=FALSE;
int timeout=DEFAULT_TIMEOUT;
//int invert_test=FALSE;
//int use_celsius=FALSE;

int model, firmware_version;

char sendbuf[BUFFER_SIZE];
char recvbuf[BUFFER_SIZE];
char *tmpbuf;
int fd;
struct termios oldtios;
struct termios newtios;
int debug_loop_done;

void BS2_comm_init(void);
void BS2_comm_halt(void);
void BS2_reset(void);
int BS2_id(void);
// void BS2_write(char * str);
void BS2_read(char * str, int count);
void BS2_sendfile(void);
void BS2_sendfile(char * fname);
void *timeout_handler(int sig);
void *control_c_handler(int sig);

int main(int argc, char **argv)
{
	int ready_for_debug=0;
	char recvchar;

	const char * bsdev = getenv("BSTAMP_DEVICE");
	if(bsdev) {
	  device = strdup(bsdev);
	}
	
	if (argc == 2)
	{
	if (strlen(argv[1])>4) // use the filename (like infile.tok) to program the Stamp
	{
		BS2_comm_init();
		if (BS2_id())
		{
			if (model!=MODEL_BS2)
			{
				unsigned char program_slot=0;
				write(fd, &program_slot, 1); // send program slot
				tcflush(fd,TCIFLUSH);
				usleep(500000);
			}
			fprintf(stderr, "Basic Stamp detected!");
			fprintf(stderr, "Model: %s\n", model_names[model]);
			fprintf(stderr, "Firmware version in BCD = %d\n", firmware_version);
			BS2_sendfile(argv[1]);
			ready_for_debug=1; // now we know the stamp is programmed and debugging
		}
		else
		{
			fprintf(stderr, "Error: No BASIC Stamp identified!\n");
			fprintf(stderr, "Probably the stamp isn't connected, perhaps your stamp version isn't supported?\n");
			fprintf(stderr, "Try looking at the help, try '%s -h' for more information.\n", argv[0]);
			exit(1);
		}
	}
	else if (argv[1][0]=='-')
	{
		if ((argv[1][1]=='h')|(argv[1][1]=='?'))
		{
			fprintf(stderr, "\nBasic Stamp Linux Programmer / Debugger\n\n");
			fprintf(stderr, "This program is used to program and run a basic stamp.\n");
			fprintf(stderr, "Before it can be used, some simple setup must be done (see below).\n");
			fprintf(stderr, "For more information about the Basic Stamp, see http://www.parallax.com\n\n");
			fprintf(stderr, "This is a modified version of bstamp available at http://bstamp.sourceforge.net\n");
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "\t-h or -? : this help listing\n");
			fprintf(stderr, "\t-v : the program version\n");
			fprintf(stderr, "\t-a : about the program (brief history)\n");
			fprintf(stderr, "\t-s : use stdin instead of INFILE.TOK\n");
			fprintf(stderr, "\talternately, to use stdin do not include any other arguments.\n\n");
			fprintf(stderr, "The basic stamp must be plugged into a local serial (COM) port.\n");
			fprintf(stderr, "Examples here use /dev/ttyS0 as the serial port,.\n");
			fprintf(stderr, " but you may need to use something else (like /dev/ttyS1).\n");
			fprintf(stderr, "A symlink named 'bstamp' in /dev must point to the com port you wish to use.\n");
			fprintf(stderr, "This can be done with 'ln -s /dev/ttyS0 /dev/bstamp'.\n");
			fprintf(stderr, "If the serial port doesn't work, you might try 'MAKEDEV /dev/ttyS0'\n");
			fprintf(stderr, "You may need to be root to accomplish this, try 'su' to become root.\n");
			fprintf(stderr, "When done with MAKEDEV and creating the symlink, type exit.\n\n");
			fprintf(stderr, "You must include a tokenized program when calling this program.\n");
			fprintf(stderr, "First run 'bstamp_tokenize INFILE.TXT OUTFILE.TOK' to make the tokenized program.\n");
			fprintf(stderr, "Next run 'cat INFILE.TOK | bstamp_run'\n\n");
			fprintf(stderr, "Standard Usage: bstamp_run INFILE.TOK\n");
			fprintf(stderr, " Or:\n cat INFILE.TOK | bstamp_run\n Or:\n cat INFILE.BS2 | bstamp_tokenizer | bstamp_run\n");
			exit(0);
		}
		else if (argv[1][1]=='v')
		{
			fprintf(stderr, "bstamp_run version 2004-05-12\n current filename: %s\n", argv[0]);
			exit(0);
		}
		else if (argv[1][1]=='a')
		{
			fprintf(stderr, "Modified by Francis Esmonde-White [francis@esmonde-white.com] 2004-05-12\n");
			fprintf(stderr, "notes: added support for BS2p, BS2pe\n");
			fprintf(stderr, "\talso rearranged the code so that future stamps can be more easily added\n");
			fprintf(stderr, "\trenamed internal functions to start with BS2_\n");
			fprintf(stderr, "\tnow using STDIN as the .TOK file location if no arguments are given\n");
			fprintf(stderr, "Stamps recognized by this version:\n");
			fprintf(stderr, "\tBS2\n");
			fprintf(stderr, "\tBS2e\n");
			fprintf(stderr, "\tBS2sx\n");
			fprintf(stderr, "\tBS2p (BS2p24 and BS2p40)\n");
			fprintf(stderr, "\tBS2pe (BS2pe24 and BS2pe40)\n");
			fprintf(stderr, "Known bug: Outputs an unexpected letter after receiving CTRL-C for shutdown.\n");
			fprintf(stderr, "TODO: implement program slot selection in programming phase, currently programming slot 0.\n");
			exit(0);
		}
		else if (argv[1][1]=='s') // use the standard input to program the stamp
		{
			BS2_comm_init();
			if (BS2_id())
			{
				fprintf(stderr, "Basic Stamp detected!");
				fprintf(stderr, "Model: %s\n", model_names[model]);
				fprintf(stderr, "Firmware version in BCD = %d\n", firmware_version);
				BS2_sendfile();
				ready_for_debug=1; // now we know the stamp is programmed and debugging
			}
			else
			{
				fprintf(stderr, "Error: No BASIC Stamp identified!\n");
				fprintf(stderr, "Probably the stamp isn't connected, perhaps your stamp version isn't supported?\n");
				fprintf(stderr, "Try looking at the help, try '%s -h' for more information.\n", argv[0]);
				exit(1);
			}
		}
		else
		{
			fprintf(stderr, "%s: -- invalid call\n", argv[0]);
			fprintf(stderr, "Try looking at the help, try '%s -h' for more information.\n", argv[0]);
			exit(0);
		}
	}
	else
	{
		fprintf(stderr, "%s: -- invalid call\n", argv[0]);
		fprintf(stderr, "Try looking at the help, try '%s -h' for more information.\n", argv[0]);
		exit(0);
	}
	}
	else if (argc==1) // use the standard input to program the stamp
	{
		BS2_comm_init();
		if (BS2_id())
		{
			if (model!=MODEL_BS2)
			{
				unsigned char program_slot=0;
				write(fd, &program_slot, 1); // send program slot
				tcflush(fd,TCIFLUSH);
				usleep(500000);
			}
			fprintf(stderr, "Basic Stamp detected!");
			fprintf(stderr, "Model: %s\n", model_names[model]);
			fprintf(stderr, "Firmware version in BCD = %d\n", firmware_version);

			//int test_read=-2;
			//test_read = read(fd, recvbuf, 18);
			//fprintf(stderr, "Data read: %d bytes\n",test_read);
			//fprintf(stderr, "String: %s", recvbuf);

			BS2_sendfile();
			ready_for_debug=1; // now we know the stamp is programmed and debugging
		}
		else
		{
			fprintf(stderr, "Error: No BASIC Stamp identified!\n");
			fprintf(stderr, "Probably the stamp isn't connected, perhaps your stamp version isn't supported?\n");
			fprintf(stderr, "Try looking at the help, try '%s -h' for more information.\n", argv[0]);
			exit(1);
		}
	}
	else
	{
		fprintf(stderr, "%s: -- invalid call\n", argv[0]);
		fprintf(stderr, "Try looking at the help, try '%s -h' for more information.\n", argv[0]);
		exit(0);
	}


  if (ready_for_debug)
  {
	fprintf(stderr, "DEBUG OUTPUT:     (Press [Control]-[C] to complete sequence)\n");
	fprintf(stderr, "_____________________________________________________________________\n");
	fflush(stderr);

	signal(SIGINT,(sighandler_t)control_c_handler);
	alarm(0);
	debug_loop_done = 0;
        do
	{
	  if (read(fd, &recvchar, 1))
	  {
	    printf("%c", recvchar);

	    /* Take into account DOS EOL: */
	    if (recvchar == '\r')
	      printf("\n");

	    fflush(stdout);
	  }

	  /*
	  // TODO !!!
	  // ADD THE ABILITY TO COMBINE STDIN FROM THE KEYBOARD TO THE DEBUGGER
	  // 	TO HAVE COMMAND-LINE DEBUGGER STAMP INTERACTION
	  // THIS CODE IS CURRENTLY NON-FUNCTIONAL. THE IDEA WAS TO USE IT AS FOLLOWS:
	  //	cat hw.bs2 | ./bstamp_tokenize | ./bstamp_run <&0
	  // TO APPEND THE STDIN TO THE bstamp_runu command.

	  if (num_read=fread(sendbuf, 1, BUFFER_SIZE, stdin)) // COMMUNICATE BACK TO STAMP
	  {
			write(fd, sendbuf, num_read);
			tcflush(fd,TCIFLUSH);
			usleep(500000);
			//BS2_read(recvbuf, 18); // read echo
	  }
	  */
	}
        while (!debug_loop_done);

	/* Complete sequence: */

	write(fd, "\0", 1);

	BS2_comm_halt();
	return 0;
  }
  else
  {
  	return 1;
  }
}

void BS2_comm_init() {
  /*
	setup timeout handlers,
	save com port settings,
	set com port settings
  */

	/* catch timeouts */
	signal(SIGALRM,(sighandler_t)timeout_handler);
	alarm(0);

	/* open COM port */
	fd=open(device,O_RDWR | O_NOCTTY);
	if(fd<0){
		fprintf(stderr, "Error: Could not open %s for reading/writing!\n",device);
		exit(STATE_CRITICAL);
	        }

	/* save current COM port settings */
	tcgetattr(fd,&oldtios);

	/* set new COM port settings */
	newtios.c_cflag=B9600 | CS8 | CLOCAL | CREAD;
	newtios.c_iflag=IGNPAR;
	newtios.c_oflag=0;
	newtios.c_lflag=0;
	newtios.c_line=0;
	newtios.c_cc[VMIN]=0;
	newtios.c_cc[VTIME]=5;
	tcsetattr(fd,TCSANOW,&newtios);

}

void BS2_comm_halt(void)
{

	fprintf(stderr, "\nShutting down communication!\n");

	/* restore original COM port settings and close it */
	tcsetattr(fd,TCSANOW,&oldtios);
	close(fd);

	/* reset alarm */
	alarm(0);
}

void BS2_reset()
{
  /*
  	DTR & Tx High for 2ms,
	DTR Low & Tx High for 36 ms,
	DTR & Tx Low for 20 ms,
	Flush buffer
  */

  	int dtr_flag;

	/* From BASIC Stamp Programming Protocol, page 5: */

	/* 1. set DTR high */
	dtr_flag = TIOCM_DTR;
	ioctl(fd, TIOCMBIS, &dtr_flag);

	/* 2. Set break condition on TX */
	ioctl(fd, TIOCSBRK, 4);

	/* 3. Pause for at least 2 ms: */
	usleep(2000);

	/* 4. Set DTR low: */
	dtr_flag = TIOCM_DTR;
	ioctl(fd, TIOCMBIC, &dtr_flag);

	/* 5. Pause for at least 36 ms: */
	usleep(36000);

	/* 6. Clear break condition on TX */
	ioctl(fd, TIOCCBRK, 4);

	/* 7. Pause for approximately 20 ms */
	usleep(20000);

	/* ... and flush receive buffer: */
	tcflush(fd,TCIFLUSH);
}

int BS2_id()
{
  /*
  	reset the stamp,
	try the various stamp recognition sequences,
	set the type and version,
	return 1 if good, 0 otherwise
  */

  	char snd;
	model = MODEL_UNKNOWN;
	firmware_version = -1;

  /* check for BS2 */
	BS2_reset();

	snd='B';
	/* Send the identification request character */
	if (write(fd,&snd,1))
	{
	#ifdef DEBUG
    		fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
	#endif
	}

	/* Enable timeout alarm: */
	alarm(timeout);

	/* Receive and discard echo: */
	BS2_read(recvbuf, 1);
	BS2_read(recvbuf, 1);

	/* Disable timeout alarm: */
	alarm(0);
	if (recvbuf[0]==char(190))
	{
		snd='S';
		/* Send the identification request character */
		if (write(fd,&snd,1))
		{
		#ifdef DEBUG
			fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
		#endif
		}

		/* Enable timeout alarm: */
		alarm(timeout);

		/* Receive and discard echo: */
		BS2_read(recvbuf, 1);
		BS2_read(recvbuf, 1);

		/* Disable timeout alarm: */
		alarm(0);
		if (recvbuf[0]==char(173))
		{
			snd='2';
			/* Send the identification request character */
			if (write(fd,&snd,1))
			{
			#ifdef DEBUG
				fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
			#endif
			}

			/* Enable timeout alarm: */
			alarm(timeout);

			/* Receive and discard echo: */
			BS2_read(recvbuf, 1);
			BS2_read(recvbuf, 1);

			/* Disable timeout alarm: */
			alarm(0);
			if (recvbuf[0]==char(206))
			{
				snd=0;
				/* Send the identification request character */
				if (write(fd,&snd,1))
				{
				#ifdef DEBUG
					fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
				#endif
				}

				/* Enable timeout alarm: */
				alarm(timeout);

				/* Receive and discard echo: */
				BS2_read(recvbuf, 1);
				BS2_read(recvbuf, 1);

				/* Disable timeout alarm: */
				alarm(0);
				firmware_version = recvbuf[0];
				model = MODEL_BS2;
			}
		}
	}

  /* end BS2 check */

  /* check for BS2e */
  if (model==MODEL_UNKNOWN)
  {
  	BS2_reset();

	snd='e';
	/* Send the identification request character */
	if (write(fd,&snd,1))
	{
	#ifdef DEBUG
    		fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
	#endif
	}

	/* Enable timeout alarm: */
	alarm(timeout);

	/* Receive and discard echo: */
	BS2_read(recvbuf, 1);
	BS2_read(recvbuf, 1);

	/* Disable timeout alarm: */
	alarm(0);
	if (recvbuf[0]=='e')
	{
		firmware_version = 10;
		model = MODEL_BS2e;
	}
  }
  /* end BS2e check */

  /* check for BS2sx */
  if (model==MODEL_UNKNOWN)
  {
  	BS2_reset();

	snd='X';
	/* Send the identification request character */
	if (write(fd,&snd,1))
	{
	#ifdef DEBUG
    		fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
	#endif
	}

	/* Enable timeout alarm: */
	alarm(timeout);

	/* Receive and discard echo: */
	BS2_read(recvbuf, 1);
	BS2_read(recvbuf, 1);

	/* Disable timeout alarm: */
	alarm(0);
	if (recvbuf[0]=='X')
	{
		firmware_version = 10;
		model = MODEL_BS2sx;
	}
	else if (recvbuf[0]=='Y')
	{
		firmware_version = 11;
		model = MODEL_BS2sx;
	}
	else if (recvbuf[0]=='Z')
	{
		firmware_version = 12;
		model = MODEL_BS2sx;
	}
  }
  /* end BS2sx check */

  /* check for BS2p */
  if (model==MODEL_UNKNOWN)
  {
  	BS2_reset();

	snd='P';
	/* Send the identification request character */
	if (write(fd,&snd,1))
	{
	#ifdef DEBUG
    		fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
	#endif
	}

	/* Enable timeout alarm: */
	alarm(timeout);

	/* Receive and discard echo: */
	BS2_read(recvbuf, 1);
	BS2_read(recvbuf, 1);

	/* Disable timeout alarm: */
	alarm(0);
	if (recvbuf[0]=='p')
	{
		firmware_version = 10;
		model = MODEL_BS2p24;
	}
	else if (recvbuf[0]=='q')
	{
		firmware_version = 11;
		model = MODEL_BS2p24;
	}
	else if (recvbuf[0]=='r')
	{
		firmware_version = 12;
		model = MODEL_BS2p24;
	}
	else if (recvbuf[0]=='s')
	{
		firmware_version = 13;
		model = MODEL_BS2p24;
	}
	else if (recvbuf[0]=='P')
	{
		firmware_version = 10;
		model = MODEL_BS2p40;
	}
	else if (recvbuf[0]=='Q')
	{
		firmware_version = 11;
		model = MODEL_BS2p40;
	}
	else if (recvbuf[0]=='R')
	{
		firmware_version = 12;
		model = MODEL_BS2p40;
	}
	else if (recvbuf[0]=='S')
	{
		firmware_version = 13;
		model = MODEL_BS2p40;
	}
  }
  /* end BS2p check */

  /* check for BS2pe */
  if (model==MODEL_UNKNOWN)
  {
  	BS2_reset();

	snd='I';
	/* Send the identification request character */
	if (write(fd,&snd,1))
	{
	#ifdef DEBUG
    		fprintf(stderr, "Write succeeded (%d = %c)\n", snd, snd);
	#endif
	}

	/* Enable timeout alarm: */
	alarm(timeout);

	/* Receive and discard echo: */
	BS2_read(recvbuf, 1);
	BS2_read(recvbuf, 1);

	/* Disable timeout alarm: */
	alarm(0);
	if (recvbuf[0]=='i')
	{
		firmware_version = 10;
		model = MODEL_BS2pe24;
	}
	else if (recvbuf[0]=='j')
	{
		firmware_version = 11;
		model = MODEL_BS2pe24;
	}
	else if (recvbuf[0]=='k')
	{
		firmware_version = 12;
		model = MODEL_BS2pe24;
	}
	else if (recvbuf[0]=='I')
	{
		firmware_version = 10;
		model = MODEL_BS2pe40;
	}
	else if (recvbuf[0]=='J')
	{
		firmware_version = 11;
		model = MODEL_BS2pe40;
	}
	else if (recvbuf[0]=='K')
	{
		firmware_version = 12;
		model = MODEL_BS2pe40;
	}
  }
  /* end BS2pe check */
  if (model==MODEL_UNKNOWN) return 0;
  else return 1;
}

/*
void BS2_write(char * str)
{
  if (write(fd,str,strlen(str)) )
  {
	#ifdef DEBUG
		fprintf(stderr, "Write succeeded [ %s ]\n", str);
	#endif
  }
  usleep(20000);
}
*/

void BS2_read(char * str, int count)
{
  int i, num_got;
  char chr;

  num_got = 0;

  for (i = 0; i < count; i++)
  {
    if (!read(fd,&chr,1))
	{
	  str[i] = '\0';
	}
	else
	{
	  str[i] = chr;
	  num_got++;
	}
  }

  str[i] = '\0';

  #ifdef DEBUG
	fprintf(stderr, "RECEIVED %d BYTES:  %s\n", num_got, str);
	for (i = 0; i < num_got; i++)
	{
	  fprintf(stderr, "%4d = %d\n", i, (unsigned char) str[i]);
	}
	fprintf(stderr, "\n");
  #endif
}

void BS2_sendfile()
{
  int num_read;
  char buf[18];
  char recvbuf[18];
  char recvchar;

if (model == MODEL_BS2)
{
	do
	{
		num_read = fread(buf, 1, 18, stdin);
		#ifdef DEBUG
		fprintf(stderr, "Assuming .TOK file on stdin\n", num_read);
		fprintf(stderr, "Read %d bytes from file...\n", num_read);
		#endif

		if (num_read > 0)
		{
			write(fd, buf, num_read);
			tcflush(fd,TCIFLUSH);
			usleep(500000);
			BS2_read(recvbuf, 18); // read echo
			BS2_read(&recvchar, 1); // ack
			fprintf(stderr, "Ack = %d\n", recvchar);
		}
	}
	while (num_read > 0);
}
else if(model!=MODEL_UNKNOWN) // BS2e, BS2sx, BS2p, BS2pe
{
	fcntl(fd,F_SETFD,O_FSYNC);
	do
	{
		num_read = fread(buf, 1, 18, stdin);
		#ifdef DEBUG
		fprintf(stderr, "Assuming .TOK file on stdin\n");
		fprintf(stderr, "Read %d bytes from .TOK file...\n", num_read);
		#endif

		if (num_read > 0)
		{

			fprintf(stderr, "%ld characters transmitted\n", TEMP_FAILURE_RETRY (write(fd, buf, num_read)));  // send 18-byte packet

			tcflush(fd,TCIFLUSH);
			usleep(500000);


			int read_sz=read(fd, recvbuf, num_read); // read and discard 18-byte packet

			fprintf(stderr, "%d characters echoed\n",read_sz);
			read_sz=read(fd, &recvchar, 1); // read ack byte


			if (recvchar==1) // handle Communication error
			{
				fprintf(stderr, "Ack = %d\n", recvchar); // print ack byte
				fprintf(stderr, "Error: Communication error while programming stamp, try again\n");
				exit(1);
			}
			else if (recvchar==2) // handle EEPROM error
			{
				fprintf(stderr, "Ack = %d\n", recvchar); // print ack byte
				fprintf(stderr, "Error: EEPROM error.\n");
				fprintf(stderr, "If this happens consistently, there may be a problem with your stamp.\n");
				exit(1);
			}
			else if (recvchar!=0)
			{
				fprintf(stderr, "Ack = %d\n", recvchar); // print ack byte
				fprintf(stderr, "Error: Unknown error while programming stamp!\n");
				exit(1);
			}
		}
	}
	while (num_read > 0);
}
else
{
	fprintf(stderr, "Error: Basic Stamp model not recognized when trying to upload\n");
	exit(1);
}

}

void BS2_sendfile(char * fname)
{
  int in_fd, num_read;
  unsigned char buf[18];
  unsigned char recvbuf[18];
  unsigned char recvchar;

if (model == MODEL_BS2)
{
	in_fd = open(fname, O_RDONLY);
	if (in_fd == -1)
	{
		fprintf(stderr, "Error loading %s!\n", fname);
		exit(1);
	}

	do
	{
		num_read = read(in_fd, buf, 18);
		#ifdef BUG
		fprintf(stderr, "Read %d bytes from file...\n", num_read);
		#endif

		if (num_read > 0)
		{
			write(fd, buf, num_read);
			tcflush(fd,TCIFLUSH);
			usleep(500000);
			read(fd, recvbuf, 18); // num_read);
			read(fd, &recvchar, 1);
			fprintf(stderr, "Ack = %d\n", recvchar);
		}
	}
	while (num_read > 0);

	close(in_fd);
}
else if(model!=MODEL_UNKNOWN) // BS2e, BS2sx, BS2p, BS2pe
{
	in_fd = open(fname, O_RDONLY);
	if (in_fd == -1)
	{
		fprintf(stderr, "Error loading %s!\n", fname);
		exit(1);
	}

	do
	{
		num_read = read(in_fd, buf, 18);
		#ifdef BUG
		fprintf(stderr, "Read %d bytes from file...\n", num_read);
		#endif

		if (num_read > 0)
		{

			fprintf(stderr, "%ld characters transmitted\n", TEMP_FAILURE_RETRY (write(fd, buf, num_read)));  // send 18-byte packet

			tcflush(fd,TCIFLUSH);
			usleep(500000);

			int read_sz=read(fd, recvbuf, num_read); // read and discard 18-byte packet

			fprintf(stderr, "%d characters echoed\n",read_sz);
			read_sz=read(fd, &recvchar, 1); // read ack byte


			if (recvchar==1) // handle Communication error
			{
				fprintf(stderr, "Ack = %d\n", recvchar); // print ack byte
				fprintf(stderr, "Error: Communication error while programming stamp, try again\n");
				exit(1);
			}
			else if (recvchar==2) // handle EEPROM error
			{
				fprintf(stderr, "Ack = %d\n", recvchar); // print ack byte
				fprintf(stderr, "Error: EEPROM error.\n");
				fprintf(stderr, "If this happens consistently, there may be a problem with your stamp.\n");
				exit(1);
			}
			else if (recvchar!=0)
			{
				fprintf(stderr, "Ack = %d\n", recvchar); // print ack byte
				fprintf(stderr, "Error: Unknown error while programming stamp!\n");
				exit(1);
			}
		}
	}
	while (num_read > 0);

	close(in_fd);
}
else
{
	fprintf(stderr, "Error: Basic Stamp model not recognized when trying to upload\n");
	exit(1);
}

}

/* handle timeouts */
void *timeout_handler(int sig){

  fprintf(stderr, "ERROR: Timed out waiting for data from %s\n", device);

  //tcsetattr(fd,TCSANOW,&oldtios);
  //close(fd);

  //exit(STATE_CRITICAL);

  return NULL;
}

/* handle control c */
void *control_c_handler(int sig)
{

	fflush(stdout);
	fflush(stderr);

	fprintf(stderr, "\n_____________________________________________________________________\n");
	fprintf(stderr, "\nReceived [Control]-[C]!\n");
	fflush(stderr);

	debug_loop_done = 1;
	return NULL;
}
