#include <iostream>
#include <string>

#include "stamp.h"

using namespace std;

stamp::stamp()
{
	vender = "";
}

stamp::~stamp()
{

}

string stamp::get_vender(void)
{
	return vender;
}

void stamp::set_devicePort(string s)
{
	parallaxStamp.set_devicePort(s);
	coridiumStamp.set_devicePort(s);
}

string stamp::get_devicePort()
{
	if (vender.compare("Parallax") == 0)
		return parallaxStamp.get_devicePort();
	else if (vender.compare("Coridium") == 0)
		return coridiumStamp.get_devicePort();
}

string stamp::get_model()
{
	if (vender.compare("Parallax") == 0)
		return parallaxStamp.get_model();
	else if (vender.compare("Coridium") == 0)
		return coridiumStamp.get_model();
}

float stamp::get_firmware()
{
	if (vender.compare("Parallax") == 0)
		return parallaxStamp.get_firmware();
	else if (vender.compare("Coridium") == 0)
		return coridiumStamp.get_firmware();
}

bool stamp::setup(void) 
{
	/* Check to see if we can initialize Parallax's stamp */
	if (!parallaxStamp.stamp_init() ){
		parallaxStamp.stamp_halt();
	} else {
		/* If we were successful, see if we can id the stamp */
		if (parallaxStamp.stamp_id()){
			vender = "Parallax";
			return 1;
		}
	}

	/* Check to see if we can initialize Coridium's stamp */
	if (!coridiumStamp.stamp_init() ) {
			coridiumStamp.stamp_halt();
	} else {
		/* If we were successfull, see if we can id the stamp */
		if (coridiumStamp.stamp_id()){
			vender = "Coridium";
			return 1;
		}
	}

	return 0;
}

void stamp::reset(void)
{

	if (vender.compare("Parallax") == 0)
		parallaxStamp.stamp_reset();
	else if (vender.compare("Coridium") == 0)
		coridiumStamp.stamp_reset();
}

bool stamp::send_file(string file, int slot) {
	if (vender.compare("Parallax") == 0)
		return parallaxStamp.stamp_sendfile(file, slot);
	else if (vender.compare("Coridium") == 0)
		return coridiumStamp.stamp_sendfile(file);
}

int stamp::get_fd()
{
	if (vender.compare("Parallax") == 0)
		return parallaxStamp.get_fd();
	else if (vender.compare("Coridium") == 0)
		return coridiumStamp.get_fd();
}

void stamp::debug_end()
{
	 if (vender.compare("Coridium") == 0){}
}

void stamp::halt(void)
{
	if (vender.compare("Parallax") == 0)
		parallaxStamp.stamp_halt();
	else if (vender.compare("Coridium") == 0)
		coridiumStamp.stamp_halt();

}
