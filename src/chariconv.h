#ifndef _ST_CHARICONV_H
#define _ST_CHARICONV_H

#include <stddef.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#endif
#include <string>

namespace chariconv {
	/**
	 * @brief Convert string from a encoding to another encoding
	 * @param input input string
	 * @param input_size the size of input string
	 * @param output output string (Need free memory mannally)
	 * @param output_size the size of output string
	 * @param ori_enc origin encoding
	 * @param des_enc dest encoding
	 * @param iconv_only only use iconv
	 * @return true if OK
	*/
	bool convert(const char* input, size_t input_size, char*& output, size_t& output_size, const char* ori_enc, const char* des_enc, bool iconv_only = false);
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Convert string from a code page to another code page
	 * @param input Input string
	 * @param input_size The size of input string
	 * @param output Output string (Need free memory mannanlly)
	 * @param output_size The size of output string
	 * @param ori_cp Origin code page
	 * @param des_cp Dest code page
	 * @return true if OK
	*/
	bool convert(const char* input, size_t input_size, char*& output, size_t& output_size, const UINT ori_cp, const UINT des_cp);
	/**
	 * @brief Convert wstring to normal string
	 * @param input Input wstring
	 * @param input_size The number of input characters, if is negative, will calling lstrlenW
	 * @param output Output string
	 * @param output_size The size of output string
	 * @param des_cp Dest code page
	 * @return true if OK
	*/
	bool convert(const wchar_t* input, int input_size, char*& output, size_t& output_size, const UINT des_cp);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Convert code page to encoding name
	 * @param cp Code page
	 * @return encoding name, may nullptr
	*/
	const char* cpToEncoding(UINT cp = CP_ACP);
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Convert encoding name to code page
	 * @param encoding Encoding name
	 * @param cp Code page
	 * @return 
	*/
	bool encodingToCp(const char* encoding, UINT& cp);
#endif
	/**
	 * @brief Compare a string with other strings
	 * @param s The compared string
	 * @param c The first string of needed compare
	 * @param count The count of more string
	 * @param  More string compare
	 * @return if the first one equal any string, return true
	*/
	bool strcmpEx(const char* s, const char* c, int count = 0, ...);
	/**
	 * @brief Convert string to lowercase. (Only safe with ASCII)
	 * @param input Input string
	 * @param input_size Input size (if set 0, will calculate it by calling strlen)
	 * @param output Output string (Need free memory mannally)
	 * @return true if OK
	*/
	bool tolowercase(const char* input, size_t input_size, char*& output);
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Convert wstring to string with UTF-8 encoding
	 * @param source Wstring
	 * @param dest Result
	 * @return true if OK
	*/
	bool WStringToUTF8(std::wstring source, std::string& dest);
#endif
}

#endif
