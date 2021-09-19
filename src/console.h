#ifndef _ST_CONSOLE_H
#define _ST_CONSOLE_H

namespace console {
	typedef enum loglevel {
		VERBOSE_LOGLEVEL,
		INFO_LOGLEVEL,
		WARNING_LOGLEVEL,
		ERROR_LOGLEVEL,
		QUIET_LOGLEVEL
	}loglevel;
	int log(loglevel level, const char* format, ...);
	int verbose(const char* format, ...);
	int info(const char* format, ...);
	int warn(const char* format, ...);
	int error(const char* format, ...);
	void set_log_level(loglevel level);
	loglevel get_log_level();
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Set Output Code Page
	 * @param cp Code Page
	 * @return true if OK
	*/
	bool setOutputCP(unsigned int cp = 65001U);
	/**
	 * @brief Reset to origin code page
	*/
	void resetOutputCP();
#endif
}

#endif
