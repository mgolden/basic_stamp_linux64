#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H

class SerialInterface
{
	private:
		int fd;
		char [] buffer;
		unsigned int bufferSize;
		
	public:
		SerialInterface();
		~SerialIntreface();
		
		void setBufferSize(int);
		unsigned int getBufferSize();
		bool open();
		bool close();
		unsigned int read(unsigned char *buf, int len);
		unsigned int write(const string s);

		// These return types are subject to change */
		void setTermios();
		void getTermios(); 

	/* Operators to overload */
	// +=, << , >> 	
};

#endif
