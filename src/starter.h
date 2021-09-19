#ifndef _ST_STARTER_H
#define _ST_STARTER_H

#include "configfile.h"
#include "cml.h"
#include <list>

class starter {
private:
	cml* cm = nullptr;
	config* conf = nullptr;
	std::list<std::string> relativefiles{};
public:
	starter(cml& c, config& cf);
	/**
	 * @brief Escape the string
	 * @param s String
	 * @return result
	*/
	std::string escape(std::string s);
	/**
	 * @brief Try to find working ffplay
	 * @return path if found, otherwise empty string
	*/
	std::string findFfplay();
	/**
	 * @brief Get autoexit option for ffplay
	 * @return autoexit option for ffplay
	*/
	std::string getAutoExit();
	/**
	 * @brief Get Relative Files
	 * @return true if OK
	*/
	bool getRelativeFiles();
	/**
	 * @brief Get all external subtitles from relative files
	 * @return vf option for ffplay
	*/
	std::string getExternalSubtitles();
	/**
	 * @brief Get width option for ffplay
	 * @return width option for ffplay
	*/
	std::string getWidth();
	/**
	 * @brief Quote string if string contains space
	 * @param s the string
	 * @return the quoted string
	*/
	static std::string quote(std::string s);
	/**
	 * @brief Start ffmpeg
	 * @return the return value
	*/
	int start();
	/**
	 * @brief test ffplay whether to work well
	 * @param path The path to call ffplay
	 * @return true if work
	*/
	static bool testFfplay(const char* path);
	/**
	 * @brief test ffplay whether to work well
	 * @param path The path to call ffplay
	 * @return true if work
	*/
	static bool testFfplay(std::string path);
};

#endif
