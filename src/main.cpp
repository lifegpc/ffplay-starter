#include "fileop.h"
#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif
#include <malloc.h>
#include "configfile.h"
#ifdef HAVE_JSONC
#include "jsonc.h"
#endif
#include "cml.h"
#include "console.h"
#include "util.h"
#include "starter.h"

int main(int argc, char *argv[]) {
#if defined(_WIN32) && !defined(__CYGWIN__)
	auto setcp = console::setOutputCP();
#endif
	cml cm(argc, argv);
	if (cm.print_help()) {
#if defined(_WIN32) && !defined(__CYGWIN__)
		if (setcp && cm.rcp) console::resetOutputCP();
#endif
		return cm.have_error() ? 1 : 0;
	}
	char* base = NULL;
	config conf;
	std::string sloc = fileop::getProgramLocation();
	if (!sloc.empty()) {
		console::verbose("Get program location: %s", sloc.c_str());
		std::string base;
		if (fileop::splitext(sloc, &base, nullptr)) {
			std::string s;
#ifdef HAVE_JSONC
			s = base + ".json";
			if (fileop::exists(s)) {
				console::verbose("Found config file \"%s\".", s.c_str());
				jsonc::read_json_file(s.c_str(), conf);
			}
#endif
		}
	}
	starter st(cm, conf);
	auto re = st.start();
	console::verbose("Ffplay returned %d.", re);
#if defined(_WIN32) && !defined(__CYGWIN__)
	if (setcp && cm.rcp) console::resetOutputCP();
#endif
	return re;
}
