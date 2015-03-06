#ifndef __DEFS_H
#define __DEFS_H

//******************************************************************************
// defs.h
//
// ASC20021014 created
// ASC20021113 added STAMPID_, COMM_
//******************************************************************************

#define byte unsigned char

#define STAMPBC_VERSION		"stampbc v1.2.4"

#define COMM_DEVICE		"/dev/ttyS"
#define COMM_DEVICE_DEFAULT	"/dev/ttyS0"

#define RECV_TIMEOUT_S          0
#define RECV_TIMEOUT_US         250000

#define RESET_DTRPULSE_US	5000

// stamp IDs - order as required by tokenizer.so
#define STAMPID_NONE		"none"
#define STAMPID_RESERVED	"reserved"
#define STAMPID_BS2		"BS2"
#define STAMPID_BS2E		"BS2e"
#define STAMPID_BS2SX		"BS2sx"
#define STAMPID_BS2P		"BS2p"
#define STAMPID_BS2PE		"BS2pe"

#define CERR			cout

// eye catchers
#ifdef _COLORFUL
#define EYEERR			"\033[31m**"
#define EYEERC			"\033[31m  "
#define EYEINF			"\033[34m--"
#define EYEINC			"\033[34m  "
#define ENDL			"\033[0m\n"
#else
#define EYEERR			"**"
#define EYEERC			"  "
#define EYEINF			"--"
#define EYEINC			"  "
#define ENDL			endl
#endif

// color sequences
#define CBLKGRN			"\033[30;42m"
#define CBLKYLW			"\033[30;43m"
#define CBLKCYN			"\033[30;46m"
#define CBLKGRY			"\033[30;47m"
#define CWHTBLK			"\033[37;40m"
#define CWHTRED			"\033[37;41m"
#define CWHTBLU			"\033[37;44m"
#define CWHTMGT			"\033[37;45m"
#define CNONE			"\033[0m"

// abstract colors
#define CEEPROGRAM		CBLKYLW
#define CEEDEFINED		CBLKCYN
#define CEEWORD			CBLKYLW
#define CEEBYTE			CBLKGRN
#define CEENIBBLE		CBLKCYN
#define CEEBIT			CBLKGRY

#endif
