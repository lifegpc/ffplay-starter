#include "fileop.h"

#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include <io.h>
#else
#include <wchar.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#ifdef HAVE_READDIR64
#define readdir readdir64
#endif

#endif
#include "util.h"
#include "chariconv.h"
#include "console.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if defined(HAVE__ACCESS_S)
#define access _access_s
#elif defined(_WIN32)
#define access _access
#endif

#ifdef HAVE__WACCESS_S
#define _waccess _waccess_s
#endif

#ifdef HAVE__FTELLI64
#define ftell _ftelli64
#elif defined(HAVE_FTELLO64)
#define ftell ftello64
#elif defined(HAVE_FTELLO)
#define ftell ftello
#endif

#ifdef HAVE__FSEEKI64
#define fseek _fseeki64
#elif defined(HAVE_FSEEKO64)
#define fseek fseeko64
#elif defined(HAVE_FSEEKO)
#define fseek fseeko
#else
#define fseek(a, b, c) fseek(a, (long)b, c)
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
FILE* fopen_internal(wchar_t* fn, wchar_t* mode) {
#ifdef HAVE__WFOPEN_S
	FILE* f;
	if (_wfopen_s(&f, fn, mode)) return NULL;
	return f;
#else
	return _wfopen(fn, mode);
#endif
}

bool exists_internal(wchar_t* fn) {
	return !_waccess(fn, 0);
}

bool fileop_internal_bool(wchar_t* fn) {
	return true;
}

FILE* popen_internal(wchar_t* fn, wchar_t* mode) {
	return _wpopen(fn, mode);
}

int system_internal(wchar_t* command) {
	return _wsystem(command);
}

bool win32FindFile_internal(wchar_t* fname, std::list<std::string>& fl) {
	WIN32_FIND_DATAW data;
	HANDLE h = FindFirstFileW(fname, &data);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	fl.clear();
	do {
		if (data.cFileName[0] == '.') continue;
		char* s;
		size_t slen;
		if (chariconv::convert(data.cFileName, min(lstrlenW(data.cFileName), 260), s, slen, CP_UTF8)) {
			fl.push_back(s);
			free(s);
		} else {
			FindClose(h);
			return false;
		}
	} while (FindNextFileW(h, &data));
	FindClose(h);
	return true;
}

template <typename T, typename ... Args>
T fileop_internal(const char* fname, UINT codePage, T(*callback)(wchar_t* fn, Args... args), T failed, Args... args) {
	int wlen;
	wchar_t* fn;
	DWORD opt = util::getMultiByteToWideCharOptions(MB_ERR_INVALID_CHARS, codePage);
	wlen = MultiByteToWideChar(codePage, opt, fname, -1, NULL, 0);
	if (!wlen) return failed;
	fn = (wchar_t*)malloc(sizeof(wchar_t) * wlen);
	if (!MultiByteToWideChar(codePage, opt, fname, -1, fn, wlen)) {
		free(fn);
		return failed;
	}
	T re = callback(fn, args...);
	free(fn);
	return re;
}
#endif

FILE* fileop::open(const char *fname, const char *mode) {
#if defined(_WIN32) && !defined(__CYGWIN__)
	if (fname == NULL || mode == NULL) return NULL;
	int wlen;
	wchar_t* wmode;
	DWORD opt = util::getMultiByteToWideCharOptions(MB_ERR_INVALID_CHARS, CP_UTF8);
	wlen = MultiByteToWideChar(CP_UTF8, opt, mode, -1, NULL, 0);
	if (wlen) {
		wmode = (wchar_t*)malloc(sizeof(wchar_t) * wlen);
		if (MultiByteToWideChar(CP_UTF8, opt, mode, -1, wmode, wlen)) {
			UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
			int i;
			for (i = 0; i < 3; i++) {
				FILE* f = fileop_internal(fname, cp[i], &fopen_internal, (FILE*)NULL, wmode);
				if (f) {
					free(wmode);
					return f;
				}
			}
		}
		free(wmode);
	}
#if HAVE_FOPEN_S
	FILE* f;
	if (fopen_s(&f, fname, mode)) return NULL;
	return f;
#else
	return fopen(fname, mode);
#endif
#else
	return fopen(fname, mode);
#endif
}

bool fileop::exists(const char* fname, bool use_wchar) {
#if defined(_WIN32) && !defined(__CYGWIN__)
	if (!use_wchar) return !access(fname, 0);
	UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
	int i;
	for (i = 0; i < 3; i++) {
		if (fileop_internal(fname, cp[i], &exists_internal, false)) return true;
	}
	return !access(fname, 0);
#else
	return !access(fname, 0);
#endif
}

bool fileop::exists(std::string fname, bool use_wchar) {
	if (fname.empty()) return false;
	return exists(fname.c_str(), use_wchar);
}

bool fileop::exists(const wchar_t* fname) {
#if defined(_WIN32) && !defined(__CYGWIN__)
	return !_waccess(fname, 0);
#else
	size_t len = wcslen(fname) + 1;
	char* str;
	if (len <= 1) return false;
	str = (char*)malloc(len * sizeof(wchar_t));
	size_t ret = wcstombs(str, fname, len * sizeof(wchar_t));
	if (ret == -1) {
		free(str);
		return false;
	}
	bool re = !access(str, 0);
	free(str);
	return re;
#endif
}

bool fileop::close(FILE* f) {
	return !fclose(f);
}

bool fileop::splitext(const std::string fname, std::string* name, std::string* ext) {
	if (fname.empty() || (!name && !ext)) return false;
	auto l1 = fname.find_last_of('.');
	auto l2 = fname.find_last_of('/');
	auto l3 = fname.find_last_of('\\');
	if ((l2 != -1 && l2 > l1) || (l3 != -1 && l3 > l1)) l1 = -1;
	if (name) {
		if (l1 == -1) *name = fname;
		else if (!l1) *name = "";
		else *name = fname.substr(0, l1);
	}
	if (ext) {
		if (l1 == -1) *ext = "";
		else if (!l1) *ext = fname;
		else *ext = fname.substr(l1, fname.length() - l1);
	}
	return true;
}

bool fileop::concat(const char* base, const char* ext, char*& path) {
	if (base == NULL || ext == NULL) return false;
	size_t len = strlen(base) + strlen(ext) + 1;
	path = (char*)malloc(len);
	if (!path) return false;
#ifdef HAVE_STRCPY_S
	if (strcpy_s(path, len, base)) {
		free(path);
		path = NULL;
		return false;
	}
#else
	strcpy(path, base);
#endif
#ifdef HAVE_STRCAT_S
	if (strcat_s(path, len, ext)) {
		free(path);
		path = NULL;
		return false;
	}
#else
	strcat(path, ext);
#endif
	return true;
}

bool fileop::filesize(FILE* f, size_t& size) {
	if (f == NULL) return false;
	long long ori = ftell(f);
	if (ori == -1) return false;
	if (fseek(f, 0, SEEK_END)) return false;
	long long tsize = ftell(f);
	if (tsize == -1) return false;
	if (fseek(f, ori, SEEK_SET)) return false;
	size = tsize;
	return true;
}

bool fileop::tell(FILE* f, size_t& loc) {
	if (f == NULL) return false;
	long long t = ftell(f);
	if (t == -1) return false;
	loc = t;
	return true;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
bool fileop::convertToUTF8(const char* input, char*& output) {
	if (!input) return false;
	UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
	int i;
	for (i = 0; i < 3; i++) {
		auto c = cp[i];
		auto re = fileop_internal(input, c, &fileop_internal_bool, false);
		if (re) {
			if (c == CP_UTF8) {
				char* s;
				auto re = util::copystr(input, 0, s, false);
				output = s;
				return re;
			}
			else {
				char* s;
				size_t len;
				if (chariconv::convert(input, strlen(input), s, len, c, CP_UTF8)) {
					output = s;
					return true;
				}
				return false;
			}
		}
	}
	return false;
}

bool fileop::getOpenFileName(char*& filename, const wchar_t* filter, const wchar_t* title, const wchar_t* initialDir, bool default_flags, unsigned long flags, unsigned long maxFileNameSize) {
	if (!util::CoInitialize()) return false;
	DWORD deflags = OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_NONETWORKBUTTON;
	DWORD fl = default_flags ? (deflags | flags) : flags;
	if (!maxFileNameSize) maxFileNameSize = DefaultMaxFileNameSize;
	if (maxFileNameSize < MAX_PATH) maxFileNameSize = MAX_PATH;
	auto s = (wchar_t*)malloc(sizeof(wchar_t) * maxFileNameSize);
	if (!s) {
		console::warn("Can not reallocate memory, needed size: %zi.", sizeof(wchar_t) * maxFileNameSize);
		return false;
	}
	s[0] = 0;
	OPENFILENAMEW data;
	data.lStructSize = sizeof(OPENFILENAMEW);
	data.hwndOwner = nullptr;
	data.lpstrFilter = filter;
	data.lpstrCustomFilter = nullptr;
	data.nFilterIndex = 0;
	data.lpstrFile = s;
	data.nMaxFile = maxFileNameSize;
	data.lpstrFileTitle = nullptr;
	data.lpstrInitialDir = initialDir;
	data.lpstrTitle = title;
	data.Flags = fl;
	data.lpstrDefExt = nullptr;
	data.FlagsEx = 0;
	auto re = GetOpenFileNameW(&data);
	if (!re) {
		free(s);
		return false;
	}
	char* ns;
	size_t nlen;
	if (chariconv::convert(s, -1, ns, nlen, CP_UTF8)) {
		filename = ns;
		free(s);
		return true;
	}
	free(s);
	return false;
}
#endif

FILE* fileop::popen(const char* command, const char* mode) {
	if (!command || !mode) return nullptr;
#if defined(_WIN32) && !defined(__CYGWIN__)
	int wlen;
	wchar_t* wmode;
	DWORD opt = util::getMultiByteToWideCharOptions(MB_ERR_INVALID_CHARS, CP_UTF8);
	wlen = MultiByteToWideChar(CP_UTF8, opt, mode, -1, NULL, 0);
	if (wlen) {
		wmode = (wchar_t*)malloc(sizeof(wchar_t) * wlen);
		if (MultiByteToWideChar(CP_UTF8, opt, mode, -1, wmode, wlen)) {
			UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
			int i;
			for (i = 0; i < 3; i++) {
				auto f = fileop_internal<FILE*, wchar_t*>(command, cp[i], &popen_internal, nullptr, wmode);
				if (f) {
					free(wmode);
					return f;
				}
			}
		}
		free(wmode);
	}
	return _popen(command, mode);
#else
	return ::popen(command, mode);
#endif
}

int fileop::pclose(FILE* f) {
	if (!f) return -1;
#if defined(_WIN32) && !defined(__CYGWIN__)
	return ::_pclose(f);
#else
	return ::pclose(f);
#endif
}

int fileop::system(const char* command) {
	if (!command) return -1;
#if defined(_WIN32) && !defined(__CYGWIN__)
	UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
	int i;
	for (i = 0; i < 3; i++) {
		auto r = fileop_internal(command, cp[i], &system_internal, INT_MIN + 1);
		if (r != (INT_MIN + 1)) {
			return r;
		}
	}
#endif
	return ::system(command);
}

std::string fileop::getProgramLocation() {
#if defined(_WIN32) && !defined(__CYGWIN__)
	auto fn = (wchar_t*)malloc(sizeof(wchar_t) * DefaultMaxFileNameSize);
	if (!fn) {
		console::warn("Can not reallocate memory, needed size: %zi.", sizeof(wchar_t) * DefaultMaxFileNameSize);
		return "";
	}
	auto len = GetModuleFileNameW(nullptr, fn, DefaultMaxFileNameSize);
	if (len) {
		char* s;
		size_t a;
		if (chariconv::convert(fn, len, s, a, CP_UTF8)) {
			std::string re(s);
			free(s);
			free(fn);
			return re;
		}
	}
	free(fn);
	return "";
#else
	auto fn = (char*)malloc(DefaultMaxFileNameSize);
	if (!fn) {
		console::warn("Can not reallocate memory, needed size: %zi.", DefaultMaxFileNameSize);
		return "";
	}
	auto len = readlink("/proc/self/exe", fn, DefaultMaxFileNameSize);
	if (len > 0) {
		std::string temp(fn);
		free(fn);
		return temp;
	}
	free(fn);
	return "";
#endif
}

#if defined(_WIN32) && !defined(__CYGWIN__)
bool fileop::win32FindFile(const char* fname, std::list<std::string>& fl) {
	if (!fname) return false;
	UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
	int i;
	for (i = 0; i < 3; i++) {
		auto re = fileop_internal<bool, std::list<std::string>&>(fname, cp[i], &win32FindFile_internal, false, fl);
		if (re) return re;
	}
	WIN32_FIND_DATAA data;
	HANDLE h = FindFirstFileA(fname, &data);
	if (h == INVALID_HANDLE_VALUE) return false;
	fl.clear();
	do {
		if (data.cFileName[0] == '.') continue;
		fl.push_back(data.cFileName);
	} while (FindNextFileA(h, &data));
	FindClose(h);
	return true;
}
#endif

std::string fileop::combilePath(std::string base, std::string name) {
	auto len = base.length();
	auto s = base;
	if (base[len - 1] != '\\' && base[len - 1] != '/') s += '/';
	s += name;
#if defined(_WIN32) && !defined(__CYGWIN__)
	util::strreplace(s, "/", "\\");
#else
	util::strreplace(s, "\\", "/");
#endif
	return s;
}

bool fileop::split(std::string path, std::string* dir, std::string* name) {
	if (path.empty() || (!dir && !name)) return false;
	auto l = path.find_last_of('/');
	auto l2 = path.find_last_of('\\');
	auto r = l != -1 && l2 != -1 ? max(l, l2) : l2 != -1 ? l2 : l != -1 ? l : -1;
	if (dir) {
		if (r == -1) *dir = "";
		else *dir = path.substr(0, r + 1);
#if defined(_WIN32) && !defined(__CYGWIN__)
		util::strreplace(*dir, "/", "\\");
#else
		util::strreplace(*dir, "\\", "/");
#endif
	}
	if (name) {
		if (r == -1) *name = path;
		else *name = path.substr(r + 1, path.length() - r - 1);
#if defined(_WIN32) && !defined(__CYGWIN__)
		util::strreplace(*name, "/", "\\");
#else
		util::strreplace(*name, "\\", "/");
#endif
	}
	return true;
}

bool fileop::addDirectoryToFileNameList(std::string base, std::list<std::string>& fl) {
	if (base.empty()) return false;
	for (auto i = fl.begin(); i != fl.end(); ++i) {
		*i = combilePath(base, *i);
	}
	return true;
}

bool fileop::listdir(std::string path, std::list<std::string>& fl, bool fullpath) {
#if defined(_WIN32) && !defined(__CYGWIN__)
	auto len = path.length();
	if (path[len - 1] != '/' && path[len - 1] != '\\') path += "\\";
	util::strreplace(path, "/", "\\");
	std::string s = path + "*";
	if (!win32FindFile(s.c_str(), fl)) {
		return false;
	}
#else
	util::strreplace(path, "\\", "/");
	auto dir = opendir(path.c_str());
	if (!dir) {
		console::verbose("opendir(%s) failed.", path.c_str());
		return false;
	}
	auto d = readdir(dir);
	while (d != nullptr) {
		if (d->d_name[0] == '.') {
			d = readdir(dir);
			continue;
		}
		fl.push_back(d->d_name);
		d = readdir(dir);
	}
	closedir(dir);
#endif
	if (fullpath) addDirectoryToFileNameList(path, fl);
	return true;
}

bool fileop::listrelative(std::string file, std::list<std::string>& fl) {
	if (file.empty()) return false;
	console::verbose("List all relative files for '%s'", file.c_str());
	std::string base;
	if (!splitext(file, &base, nullptr)) {
		console::verbose("Can not split file name by ext: %s", file.c_str());
		return false;
	}
	std::string path;
	if (!split(file, &path, nullptr)) {
		console::verbose("Can not split file name: %s", file.c_str());
		return false;
	}
	base += ".";
#if defined(_WIN32) && !defined(__CYGWIN__)
	auto t = base + "*";
	if (win32FindFile(t.c_str(), fl)) {
		addDirectoryToFileNameList(path, fl);
		for (auto i = fl.begin(); i != fl.end(); ++i) {
			auto fn = *i;
			if (fn == file) {
				fl.remove(fn);
				break;
			}
		}
		return true;
	}
	console::verbose("Failed to use FindFile directly.");
#endif
	std::list<std::string> tl;
	if (!listdir(path, tl)) {
		console::verbose("Can not list dir: %s", path.c_str());
		return false;
	}
	for (auto i = tl.begin(); i != tl.end(); ++i) {
		auto fn = *i;
		if (fn.find(base, 0) == 0 && fn != file) {
			fl.push_back(fn);
		}
	}
	return true;
}

bool fileop::filterFileListByExt(std::list<std::string> fl, std::list<std::string> exts, std::list<std::string>& result, bool filter_no_ext) {
	if (fl.empty() || exts.empty()) return false;
	result.clear();
	for (auto i = fl.begin(); i != fl.end(); ++i) {
		auto fn = *i;
		std::string ext;
		if (!splitext(fn, nullptr, &ext)) {
			console::verbose("Can not split file name by ext: %s", fn.c_str());
			return false;
		}
		bool found = false;
		if (!ext.empty()) {
			for (auto j = exts.begin(); j != exts.end(); ++j) {
				auto ex = "." + *j;
				if (ex == ext) {
					found = true;
					break;
				}
			}
		} else {
			if (!filter_no_ext) found = true;
		}
		if (found) result.push_back(fn);
	}
	return true;
}
