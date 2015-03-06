#!/bin/bash

setColor() { echo -ne $1; }
setOption() { option="$speed $default_option $1"; }
setProgOption()
{
	if [ $bspx == 1 ]; then
		speed="19200"
	else
		speed="9600"
	fi
	setOption $prog_option
	echo set prog options
}
setDebugOption()
{
	if [ $bspx == 1 ]; then
		speed="38400"
	else
		speed="9600"
	fi
	setOption $debug_option
	echo set debug options
}

opendebug=0
bspx=0
option=""
speed="9600"

default_option="raw -echo -hupcl"
debug_option="icrnl"
prog_option="-icrnl"

device=${1:-"/dev/ttyS0"}
#term=${2:-"noterm"}
term=${2:-"xterm"}

if [ $term != "noterm" ]; then
	# restart script from within an xterm
	xterm +sb -bg black -fg white -e $0 $device noterm &
	exit
fi

green="\033[32m"
white="\033[37m"
cyan="\033[36m"
yellow="\033[33m"
red="\033[31m"

while [ 1 ]; do
	clear

	if [ "$option" == "quit" ]; then
		exit
	fi

	if [ "$option" == "help" ]; then
		option=""
		stty --help | less
		clear
	fi

	if [ "$option" == "g" ]; then
		option="-g"
	fi

	setColor $yellow
	echo "device: "$device

	setColor $white
	echo '--------------------------------------------------------------------------------'

	echo -ne $red
	# BS2, BS2e, BS2sx, BS2p, BS2pe
	if [ "$option" == "bs" ]; then
		echo set BS2, BS2e, BS2sx, BS2p, BS2pe default options
		bspx=0
		setProgOption

	# BS2px
	elif [ "$option" == "bspx" ]; then
		echo set BS2px default options
		bspx=1
		setProgOption

	elif [ "$option" == "debug" ]; then
		opendebug=1
		setDebugOption

	elif [ "$option" == "d" ]; then
		setDebugOption

	elif [ "$option" == "p" ]; then
		setProgOption
	fi

	if [ "$option" != "" ]; then
		stty -F $device $option
	fi

	setColor $cyan
	stty -F $device

	setColor $white
	echo '--------------------------------------------------------------------------------'

	setColor $green
	stty -F $device -a

	if [ $opendebug == 1 ]; then
		opendebug=0
		xterm +sb -title "debug $device" -bg black -fg white \
			-e "cat $device" &
	fi

	setColor $white
	echo -ne "\n>>" && read option
done

