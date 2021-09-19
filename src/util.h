#ifndef _ST_UTIL_H
#define _ST_UTIL_H

#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PCRE
#include "pcre.h"
#endif

#include <string>

namespace util {
#ifdef HAVE_PCRE
	/**
	 * @brief Compile pcre Object
	 * @param regex Regex string
	 * @param options pcre_compile options
	 * @param tableptr pcre_compile table_ptr
	 * @return true if OK
	*/
	bool comppcre(const char* regex, pcre*& result, int options = 0, const unsigned char* tableptr = nullptr);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Initializes the COM library for use by the calling thread
	 * @param dwCoInit The concurrency model and initialization options for the thread.
	 * @return true if OK or already initialized
	*/
	bool CoInitialize(unsigned long dwCoInit = 0);
#endif
	/**
	 * @brief Copy string
	 * @param input Input string
	 * @param len The size of input string, if is 0, will calculate size by calling strlen
	 * @param output Output string
	 * @param exit_if_len_is_zero return false if len is 0.
	 * @return true if OK
	*/
	bool copystr(const char* input, size_t len, char*& output, bool exit_if_len_is_zero = true);
	/**
	 * @brief Extract UInt from string
	 * @param buff the string buffer
	 * @param start the start location
	 * @param end the end location
	 * @param result Result UInt
	 * @return true if OK
	*/
	bool extractUInt(char* buff, int start, int end, unsigned int& result);
	/**
	 * @brief Extract String from string
	 * @param buff the string buffer
	 * @param start the start location
	 * @param end the end location
	 * @param result Result string (Need free memory mannlly)
	 * @return true if OK
	*/
	bool extractString(char* buff, int start, int end, char*& result);
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Get correct options for MultiByteToWideChar
	 * @param ori_options Origin options
	 * @param cp Code Page
	 * @return Result options
	*/
	unsigned long getMultiByteToWideCharOptions(const unsigned long ori_options, unsigned int cp);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Get correct options for WideCharToMultiByte
	 * @param ori_options Origin options
	 * @param cp Code Page
	 * @return Result options
	*/
	unsigned long getWideCharToMultiByteOptions(const unsigned long ori_options, unsigned int cp);
#endif
	/**
	 * @brief Converts an integer to a string.
	 * @param i integer
	 * @param radix The base to use for the conversion of value, which must be in the range 2-36.
	 * @return string
	*/
	std::string itoa(int i, int radix = 10);
	/**
	 * @brief Replace string
	 * @param str The string want to replace
	 * @param pattern search pattern
	 * @param value replace value
	*/
	void strreplace(std::string& str, std::string pattern, std::string value);
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Get unicode version argv by using win api
	 * @param argv Result argv (need free memory by calling freeArgv.
	 * @param argc argc
	 * @return true if OK
	*/
	bool getArgv(char**& argv, int& argc);
	void freeArgv(char** argv, int argc);
#endif
}
#endif
