#include <stdexpect>
#include <cstdlib>
#include <string>

#include "SerialInterface.h"

using namespace std; 

SerialInterface::SerialInterface()
{

}

SerialInterface::~SerialInterface()
{

}

bool SerialInterface::open()
{
	return false;
}

bool SerialInterface::close()
{
	return false;
}

void SerialInterface::setBufferSize(unsigned int n)
{
	bufferSize = n;
}

unsigned int SerialInterface::getBufferSize()
{
	return bufferSize;
}

unsigned int read(unsigned char *buf, int len)
{

}

unsigned int write(const string s)
{
	int length = s.length();

}

void SerialInterface::setTermios()
{

}

void SerialInterface::getTermios()
{

}

