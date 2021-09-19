#include "cml.h"
#include "getopt.h"
#include "console.h"
#include <string.h>
#include "fileop.h"
#include "util.h"

cml::cml(int argc, char** argv) {
	struct option opts[] = { {"help", 0, nullptr, 'h'},
		{"verbose", 0, nullptr, 'v'},
		{"quiet", 0, nullptr, 'q'},
#if defined(_WIN32) && !defined(__CYGWIN__)
#define RECOVERY_OUTPUT_CP 129
		{"rcp", 0, nullptr, RECOVERY_OUTPUT_CP},
#endif
		nullptr};
	int c;
	const char* shortopts = "-:hvq";
#if defined(_WIN32) && !defined(__CYGWIN__)
	bool has_wu = false;
	int argcu;
	char** argvu;
	if (util::getArgv(argvu, argcu)) {
		has_wu = true;
		argc = argcu;
		argv = argvu;
	}
#endif
	while ((c = getopt_long(argc, argv, shortopts, opts, nullptr)) != -1) {
		switch (c)
		{
		case 'v':
			console::set_log_level(console::VERBOSE_LOGLEVEL);
			break;
		case 'q':
			console::set_log_level(console::QUIET_LOGLEVEL);
			break;
		case 'h':
			console::verbose("Get need print help message from command line.");
			help = true;
			break;
		case ':':
			help = true;
			has_error = true;
			break;
#if defined(_WIN32) && !defined(__CYGWIN__)
		case RECOVERY_OUTPUT_CP:
			console::verbose("Will recovery origin output code page after the program execute.");
			rcp = true;
			break;
#endif
		case 1:
			if (optarg && strlen(optarg)) {
				if (filename.empty()) {
#if defined(_WIN32) && !defined(__CYGWIN__)
					char* s = nullptr;
					if (!has_wu && fileop::convertToUTF8(optarg, s)) {
						filename = s;
						free(s);
					} else {
						filename = optarg;
					}
#else
					filename = optarg;
#endif
				}
				else {
					console::error("Only accept one file name: \"%s\"", optarg);
					has_error = true;
					help = true;
				}
			}
			break;
		case '?':
		default:
			if (optind <= argc && !strncmp(argv[optind - 1], "--", 2)) console::error("ffplay-starter: Unknown option -- %s", argv[optind - 1] + 2);
			else console::error("ffplay-starter: Unknown option -- %c", optopt);
			help = true;
			has_error = true;
			break;
		}
		if (has_error) break;
	}
#if defined(_WIN32) && !defined(__CYGWIN__)
	if (has_wu) util::freeArgv(argvu, argcu);
	if (filename.empty() && !help) {
		char* fn;
		if (fileop::getOpenFileName(fn, L"All Files\0*\0\0", L"Open Video/Audio File")) {
			filename = fn;
			console::info("Get video/audio file name from open dialog: %s", fn);
			free(fn);
		} else {
			console::error("File name is needed.");
			has_error = true;
			help = true;
		}
	}
#else
	if (filename.empty() && !help) {
		console::error("File name is needed.");
		has_error = true;
		help = true;
	}
#endif
	if (!filename.empty()) {
		console::verbose("Get video/audio file name: %s", filename.c_str());
	}
}

bool cml::print_help() {
	if (help) {
		auto level = has_error ? console::WARNING_LOGLEVEL : console::INFO_LOGLEVEL;
		console::log(level, "ffplay-starter [options] <filename>");
		if (has_error) {
			console::warn("Use -h/--help to see full help information.");
		} else {
			console::info("Available options:\n\
-v	--verbose	Enable verbose logging.\n\
-q	--quiet		Be quiet.\n");
#if defined(_WIN32) && !defined(__CYGWIN__)
			console::info("\
	--rcp		Recovery origin output code page after the program execute.");
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
			console::info("If filename is not given, will open a \"open file\" dialog.");
#endif
		}
	}
	return help;
}

bool cml::have_error() {
	return has_error;
}
