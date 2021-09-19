#include "console.h"
#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <string.h>
#include <malloc.h>
#endif

#ifdef HAVE_VPRINTF_S
#define vprintf vprintf_s
#endif
#ifdef HAVE_VFPRINTF_S
#define vfprintf vfprintf_s
#endif
#ifdef HAVE_PRINTF_S
#define printf printf_s
#endif
#ifdef HAVE_FPRINTF_S
#define fprintf fprintf_s
#endif

console::loglevel internal_level = console::INFO_LOGLEVEL;
#if  defined(_WIN32) && !defined(__CYGWIN__)
bool alreay_try_set_console_mode = false;
#endif

int internal_log(console::loglevel level, const char* format, va_list li) {
	if (level < internal_level) return -1;
#if  defined(_WIN32) && !defined(__CYGWIN__)
	if (!alreay_try_set_console_mode) {
		alreay_try_set_console_mode = true;
		auto h = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mod = 0;
		if (GetConsoleMode(h, &mod)) {
			if (!(mod & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
				mod = mod | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				SetConsoleMode(h, mod);
			}
		}
		auto h2 = GetStdHandle(STD_ERROR_HANDLE);
		mod = 0;
		if (GetConsoleMode(h2, &mod)) {
			if (!(mod & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
				mod = mod | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
				SetConsoleMode(h2, mod);
			}
		}
	}
#endif
	int re = -1;
	size_t len = strlen(format) + 2;
	char* new_format = (char*)malloc(len);
	if (!new_format) return -1;
#ifdef HAVE_STRCPY_S
	if (strcpy_s(new_format, len, format)) {
		free(new_format);
		return -1;
	}
#else
	strcpy(new_format, format);
#endif
	if (new_format[len - 3] != '\n') {
#ifdef HAVE_STRCAT_S
		if (strcat_s(new_format, len, "\n")) {
			free(new_format);
			return -1;
		}
#else
		strcat(new_format, "\n");
#endif
	}
	if (level <= console::INFO_LOGLEVEL) {
		bool color = internal_level == console::VERBOSE_LOGLEVEL && level == console::INFO_LOGLEVEL;
		if (color) printf("\033[1;32m");
		re = vprintf(new_format, li);
		if (color) printf("\033[0m");
	}
	if (level == console::WARNING_LOGLEVEL) {
		fprintf(stderr, "\033[1;33m");
		re = vfprintf(stderr, new_format, li);
		fprintf(stderr, "\033[0m");
	}
	if (level == console::ERROR_LOGLEVEL) {
		fprintf(stderr, "\033[1;31m");
		re = vfprintf(stderr, new_format, li);
		fprintf(stderr, "\033[0m");
	}
	free(new_format);
	return re;
}

int console::verbose(const char* format, ...) {
	int re = -1;
	va_list li;
	va_start(li, format);
	re = internal_log(VERBOSE_LOGLEVEL, format, li);
	va_end(li);
	return re;
}

int console::log(loglevel level, const char* format, ...) {
	int re = -1;
	va_list li;
	va_start(li, format);
	re = internal_log(level, format, li);
	va_end(li);
	return re;
}

int console::info(const char* format, ...) {
	int re = -1;
	va_list li;
	va_start(li, format);
	re = internal_log(INFO_LOGLEVEL, format, li);
	va_end(li);
	return re;
}

int console::warn(const char* format, ...) {
	int re = -1;
	va_list li;
	va_start(li, format);
	re = internal_log(WARNING_LOGLEVEL, format, li);
	va_end(li);
	return re;
}

int console::error(const char* format, ...) {
	int re = -1;
	va_list li;
	va_start(li, format);
	re = internal_log(ERROR_LOGLEVEL, format, li);
	va_end(li);
	return re;
}

void console::set_log_level(loglevel level) {
	internal_level = level;
}

console::loglevel console::get_log_level() {
	return internal_level;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
static UINT* origin_console_output_cp = nullptr;

bool console::setOutputCP(unsigned int cp) {
	UINT ocp = GetConsoleOutputCP();
	if (ocp == CP_ACP) ocp = GetACP();
	if (ocp == CP_OEMCP) ocp = GetOEMCP();
	if (!origin_console_output_cp) {
		origin_console_output_cp = (UINT*)malloc(sizeof(UINT));
		if (!origin_console_output_cp) {
			warn("Can not reallocate memory, needed size: %zi.", sizeof(UINT));
			return false;
		}
		*origin_console_output_cp = ocp;
	}
	return SetConsoleOutputCP(cp);
}

void console::resetOutputCP() {
	if (origin_console_output_cp) {
		SetConsoleOutputCP(*origin_console_output_cp);
	}
}
#endif
