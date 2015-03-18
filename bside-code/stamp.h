#ifndef _STAMP_H
#define _STAMP_H

#include "Parallax.h"
#include "Coridium.h"

class stamp:public Parallax,public Coridium
{

	private:
		std::string vender;
		Parallax parallaxStamp;
		Coridium coridiumStamp;
	
	public:
		stamp();
		~stamp();
		std::string get_model();
		bool setup(void);
		void reset(void);
		bool send_file(std::string, int);
		void halt(void);
		float get_firmware(void);
		int get_fd(void);
		void set_devicePort(std::string);
		std::string get_vender(void);
		std::string get_devicePort(void);
		void debug_end(void);
};
#endif
