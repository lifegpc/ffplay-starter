#include "util.h"
#include <malloc.h>
#include "console.h"
#include <string.h>
#include <stdio.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#endif
#include <list>
#include "chariconv.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef HAVE_SSCANF_S
#define sscanf sscanf_s
#endif

bool stringToChar(std::string input, char*& output) {
	auto sz = input.size();
	auto s = (char*)malloc(sz + 1);
	if (!s) {
		console::warn("Can not allocate memory, needed memory: %zi", sz + 1);
		return false;
	}
	memcpy(s, input.c_str(), sz);
	s[sz] = 0;
	output = s;
	return true;
}

void freeChar(char* input) {
	if (input) free(input);
}

template <typename T, typename R>
bool listToPointer(std::list<T> li, R*& result, bool (*convert)(T input, R& output), void (*free)(R input)) {
	if (!convert) return false;
	auto sz = li.size();
	if (sz <= 0) return false;
	R* r = (R*)malloc(sz * sizeof(void*));
	if (!r) {
		console::warn("Can not allocate memory, needed memory: %zi", sz * sizeof(void*));
		return false;
	}
	size_t j = 0;
	for (auto i = li.begin(); i != li.end(); ++i) {
		auto s = *i;
		R t;
		if (!convert(s, t)) {
			if (!free) {
				for (size_t z = 0; z < j; z++) {
					free(r[z]);
				}
			}
			::free(r);
			return false;
		}
		r[j] = t;
		j++;
	}
	result = r;
	return true;
}

template <typename T>
void freePointerList(T* li, size_t count, void (*free)(T)) {
	if (!li) return;
	for (size_t i = 0; i < count; i++) {
		if (!free) ::free(li[i]);
		else free(li[i]);
	}
	::free(li);
}

#ifdef HAVE_PCRE
bool util::comppcre(const char* regex, pcre*& result, int options, const unsigned char* tableptr) {
	if (!regex) return false;
	const char* errptr = "";
	int erroffset = 0;
	auto temp = pcre_compile(regex, options, &errptr, &erroffset, tableptr);
	if (temp) {
		result = temp;
		return true;
	}
	console::error("Compile regex pattern failed at offset %d: %s.", erroffset, errptr);
	return false;
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
static bool CoInitialized = false;

bool util::CoInitialize(unsigned long dwCoInit) {
	if (CoInitialized) return true;
	if (CoInitializeEx(nullptr, dwCoInit) == S_OK) {
		CoInitialized = true;
		return true;
	}
	return false;
}
#endif

bool util::copystr(const char* input, size_t len, char*& output, bool exit_if_len_is_zero) {
	if (!input) return false;
	if (!len) len = strlen(input);
	if (!len && exit_if_len_is_zero) return false;
	char* s = (char*)malloc(len + 1);
	if (!s) {
		console::warn("Can not allocate memory, needed size: %zi.", len + 1);
		return false;
	}
	if (len > 0) memcpy(s, input, len);
	s[len] = 0;
	output = s;
	return true;
}

bool util::extractUInt(char* buff, int start, int end, unsigned int& result) {
	if (!buff) return false;
	char* s;
	if (!extractString(buff, start, end, s)) return false;
	unsigned int t;
	if (sscanf(s, "%u", &t) > 0) result = t;
	free(s);
	return true;
}

bool util::extractString(char* buff, int start, int end, char*& result) {
	if (!buff && end <= start) return false;
	int len = end - start;
	char* s = (char*)malloc(len + 1);
	if (!s) {
		console::warn("Can not allocate memory, needed size: %zi.", len + 1);
		return false;
	}
	memcpy(s, buff + start, len);
	s[len] = 0;
	result = s;
	return true;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
unsigned long util::getMultiByteToWideCharOptions(const unsigned long ori_options, unsigned int cp) {
	if (cp == CP_ACP) cp = GetACP();
	if (cp == CP_OEMCP) cp = GetOEMCP();
	switch (cp)
	{
	case 50220:
	case 50221:
	case 50222:
	case 50225:
	case 50227:
	case 50229:
	case CP_UTF7:
	case 42:
		return 0;
	default:
		break;
	}
	if (cp >= 57002 && cp <= 57011) return 0;
	if (cp == CP_UTF8 || cp == 54936) {
		return MB_ERR_INVALID_CHARS & ori_options;
	}
	return ori_options;
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
unsigned long util::getWideCharToMultiByteOptions(const unsigned long ori_options, unsigned int cp) {
	if (cp == CP_ACP) cp = GetACP();
	if (cp == CP_OEMCP) cp = GetOEMCP();
	switch (cp)
	{
	case 50220:
	case 50221:
	case 50222:
	case 50225:
	case 50227:
	case 50229:
	case CP_UTF7:
	case 42:
		return 0;
	default:
		break;
	}
	if (cp >= 57002 && cp <= 57011) return 0;
	if (cp == CP_UTF8 || cp == 54936) {
		return WC_ERR_INVALID_CHARS & ori_options;
	}
	return ori_options;
}
#endif

std::string util::itoa(int i, int radix) {
	if (radix < 2 || radix > 36) radix = 10;
#define MAX_SIZE (sizeof(int) * 8 + 2)
	char fn[MAX_SIZE];
#ifdef HAVE__ITOA_S
	if (radix == 10) {
		if (!_itoa_s(i, fn, 10)) {
			return fn;
		}
	}
#elif defined(HAVE_ITOA)
	if (radix == 10) {
		::itoa(i, fn, 10);
		return fn;
	}
#endif
	int j = MAX_SIZE;
	auto nega = i < 0;
	if (nega) i = -i;
	fn[--j] = 0;
	do {
		int t = i % radix;
		fn[--j] = t >= 10 ? 'A' + t - 10 : '0' + t;
		i /= radix;
	} while (i > 0);
	if (nega) fn[--j] = '-';
	return std::string(fn + j);
#undef MAX_SIZE
}

void util::strreplace(std::string& str, std::string pattern, std::string value) {
	auto loc = str.find(pattern, 0);
	auto len = pattern.length();
	auto len2 = value.length();
	while (loc != -1) {
		str.replace(loc, len, value);
		if (loc + len2 < str.length()) loc = str.find(pattern, max(0, loc + len2));
		else break;
	}
}

#if defined(_WIN32) && !defined(__CYGWIN__)
/**
 * @brief Convert wstring version argv to UTF-8 encoding argv
 * @param ArgvW Source
 * @param argc The count of parameters
 * @param argv dest (Need free memory by calling freeArgv)
 * @return true if OK
*/
bool ArgvWToArgv(const LPWSTR* ArgvW, int argc, char**& argv) {
	if (!ArgvW) return false;
	std::list<std::string> li;
	for (int i = 0; i < argc; i++) {
		if (!ArgvW[i]) return false;
		std::wstring s = ArgvW[i];
		std::string r;
		if (!chariconv::WStringToUTF8(s, r)) return false;
		li.push_back(r);
	}
	char** t;
	if (!listToPointer<std::string, char*>(li, t, &stringToChar, &freeChar)) {
		return false;
	}
	argv = t;
	return true;
}

bool util::getArgv(char**& argv, int& argc) {
	auto cm = GetCommandLineW();
	int argcw = 0;
	auto argvw = CommandLineToArgvW(cm, &argcw);
	if (!argvw) return false;
	char** t;
	if (!ArgvWToArgv(argvw, argcw, t)) {
		LocalFree(argvw);
		return false;
	}
	LocalFree(argvw);
	argc = argcw;
	argv = t;
	return true;
}

void util::freeArgv(char** argv, int argc) {
	freePointerList<char*>(argv, argc, nullptr);
}
#endif
