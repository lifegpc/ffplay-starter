#ifndef _ST_CONFIGFILE_H
#define _ST_CONFIGFILE_H

#include <string>

class config {
public:
	std::string ffplay = "";
	int width = -1;
	bool autoExit = false;
};

#endif
