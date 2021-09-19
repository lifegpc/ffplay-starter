#ifndef _ST_CML_H
#define _ST_CML_H

#include <string>

class cml {
private:
	bool help = false;
	bool has_error = false;
public:
	std::string filename = "";
	cml(int argc, char** argv);
	/**
	 * \brief print help if help is needed.
	 * \return true if print help message
	*/
	bool print_help();
	bool have_error();
#if defined(_WIN32) && !defined(__CYGWIN__)
	bool rcp = false;
#endif
};

#endif
