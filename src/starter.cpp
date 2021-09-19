#include "starter.h"
#include <string>
#include "fileop.h"
#include "console.h"
#include "util.h"

starter::starter(cml& c, config& cf) {
	cm = &c;
	conf = &cf;
	relativefiles.clear();
}

std::string starter::escape(std::string s) {
	auto t = s;
	auto npos = std::string::npos;
	if (t.find(',') != npos || t.find('[') != npos || t.find(']') != npos || t.find(';') != npos) {
		util::strreplace(t, "'", R"(\')");
		t = "'" + t + "'";
		util::strreplace(t, R"(\)", R"(\\)");
		util::strreplace(t, ":", R"(\:)");
	} else {
		util::strreplace(t, "'", R"(\\\')");
		util::strreplace(t, R"(\)", R"(\\\\)");
		util::strreplace(t, ":", R"(\\:)");
	}
	return t;
}

std::string starter::findFfplay() {
	if (conf && !conf->ffplay.empty()) {
		if (testFfplay(conf->ffplay)) {
			return conf->ffplay;
		}
	}
	if (testFfplay("ffplay")) {
		return "ffplay";
	}
	return "";
}

std::string starter::getAutoExit() {
	if (conf && conf->autoExit) {
		return " -autoexit";
	}
	return "";
}

std::string starter::getExternalSubtitles() {
	std::list<std::string> subs;
	if (fileop::filterFileListByExt(relativefiles, { "ass" }, subs)) {
		std::string c = "";
		for (auto i = subs.begin(); i != subs.end(); ++i) {
			auto sub = *i;
			console::info("Add external subtitles: %s", sub.c_str());
			c += " -vf " + quote("subtitles=" + escape(sub));
		}
		return c;
	}
	console::verbose("Can not filter relative files for subtitles.");
	return "";
}

bool starter::getRelativeFiles() {
	if (cm && !cm->filename.empty()) return fileop::listrelative(cm->filename, relativefiles);
	return false;
}

std::string starter::getWidth() {
	if (conf && conf->width > 0) {
		return " -x " + util::itoa(conf->width);
	}
	return "";
}

std::string starter::quote(std::string s) {
	if (s.find_first_of(' ') != std::string::npos) {
#if defined(_WIN32) && !defined(__CYGWIN__)
		return "\"" + s + "\"";
#else
		return "'" + s + "'";
#endif
	}
	return s;
}

int starter::start() {
	getRelativeFiles();
	auto ffplay = findFfplay();
	if (ffplay.empty()) {
		console::error("Can not find ffplay.");
		return -1;
	}
	console::info("Find working ffplay: %s", ffplay.c_str());
	auto s = quote(ffplay);
	if (!cm) return -1;
	s += getAutoExit();
	s += getExternalSubtitles();
	s += getWidth();
	s += " " + quote(cm->filename);
	console::verbose("Start command line: %s", s.c_str());
	console::info("Starting ffplay.");
	return fileop::system(s.c_str());
}

bool starter::testFfplay(const char* path) {
	if (!path) return false;
	return testFfplay(std::string(path));
}

bool starter::testFfplay(std::string path) {
	auto s = quote(path);
	console::verbose("Try to find ffplay: %s", path.c_str());
	s += " -h 2>&0";
	console::verbose("Test command line: %s", s.c_str());
	auto f = fileop::popen(s.c_str(), "rb");
	if (f) {
		fileop::pclose(f);
		console::verbose("Find ffplay: %s", path.c_str());
		return true;
	}
	console::verbose("Ffplay not find: %s", path.c_str());
	return false;
}
