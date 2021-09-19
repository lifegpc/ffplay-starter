#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif
#include "jsonc.h"
#include "json-c/json.h"
#include "fileop.h"
#include <string.h>
#include "console.h"
#ifdef HAVE_CHARDET
#include "chardet.h"
#endif
#include "chariconv.h"

#define MAX_SIZE_ALLOW (2 * 1024 * 1024)

bool string_callback(json_object* obj, std::string& str) {
	if (obj == nullptr) return false;
	if (json_object_get_type(obj) != json_type_string) return false;
	auto s = json_object_get_string(obj);
	if (s == nullptr) return false;
	str = s;
	return true;
}

bool int_callback(json_object* obj, int& num) {
	if (obj == nullptr) return false;
	if (json_object_get_type(obj) != json_type_int) return false;
	auto s = json_object_get_int(obj);
	if (s == 0 && errno == EINVAL) return false;
	num = s;
	return true;
}

bool bool_callback(json_object* obj, bool& num) {
	if (obj == nullptr) return false;
	if (json_object_get_type(obj) != json_type_boolean) return false;
	auto s = json_object_get_boolean(obj);
	num = s;
	return true;
}

template<typename ... Args>
bool read_json_object(json_object* obj, const char* key, bool(*callback)(json_object* obj, Args... args), Args... args) {
	if (obj == nullptr) return false;
	json_object* t = nullptr;
	if (!json_object_object_get_ex(obj, key, &t)) return false;
	if (t == nullptr) return false;
	return callback(t, args...);
}

int jsonc::read_json_file(const char* fname, config& conf) {
	FILE* f = fileop::open(fname, "rb");
	if (!f) {
		console::warn("Can not open config file \"%s\".", fname);
		return 1;
	}
	size_t size;
	if (!fileop::filesize(f, size)) {
		console::warn("Can not get the file size of \"%s\".", fname);
		fileop::close(f);
		return 1;
	}
	if (size > MAX_SIZE_ALLOW) {
		console::warn("The file size of \"%s\" is too big. File size: %zi.\nPlease use config file which size is smaller than %i.", fname, size, MAX_SIZE_ALLOW);
		fileop::close(f);
		return 1;
	}
	char* buf = (char*)malloc(size + 1);
	/*Origin location of buf. buf may move because of bom detecting.*/
	char* ori_buf = buf;
	if (!buf) {
		console::warn("Can not allocate memory, needed size: %zi.", size + 1);
		fileop::close(f);
		return 1;
	}
	if (!fread(buf, 1, size, f)) {
		console::warn("Can not read data from config file \"%s\".", fname);
		free(buf);
		fileop::close(f);
		return 1;
	}
	buf[size] = 0;
	fileop::close(f);
	console::verbose("Open and read json config file \"%s\" successfully.", fname);
#ifdef HAVE_CHARDET
	char* encoding = nullptr;
	float confidence = 0;
	short bom = -1;
	console::verbose("Try use libchardet to detect file encoding.");
	if (chardet::det(buf, size, encoding, &confidence, bom)) {
		console::verbose("The detect result:\nencoding: %s\nconfidence: %f\nbom: %hi", encoding, confidence, bom);
		if (bom > 0) {
			int step = 0;
			if (!strcmp(encoding, "UTF-8")) step = 3;
			if (!strncmp(encoding, "UTF-16", 6)) step = 2;
			if (step > 0) {
				buf += step;
				size -= step;
				console::verbose("Skip %i chars.", step);
			}
		}
		if (!strcmp(encoding, "ASCII") || !strcmp(encoding, "UTF-8")) {
			console::verbose("Skip convert because the encoding is %s", encoding);
		} else {
#ifndef HAVE_ICONV
#if defined(_WIN32) && !defined(__CYGWIN__)
			console::info("This build don't have iconv support, will try to use win32 API to convert file encoding to UTF-8.");
#else
			console::warn("Warning: This build don't have iconv support, but json parser need UTF-8 file encoding.\nPlease save config file to UTF-8.")
#endif
#endif
			char* new_str = nullptr;
			size_t new_strl = 0;
			if (chariconv::convert(buf, size, new_str, new_strl, encoding, "UTF-8")) {
				if (new_str) {
					free(ori_buf);
					ori_buf = buf = new_str;
					size = new_strl;
				}
			}
#if defined(_WIN32) && !defined(__CYGWIN__)
			else {
				console::verbose("Try default ANSI encoding.");
				const char* enc = chariconv::cpToEncoding();
				if (enc && chariconv::convert(buf, size, new_str, new_strl, enc, "UTF-8", true)) {
					if (new_str) {
						free(ori_buf);
						ori_buf = buf = new_str;
						size = new_strl;
					}
				}
				else if (chariconv::convert(buf, size, new_str, new_strl, CP_ACP, CP_UTF8)) {
					if (new_str) {
						free(ori_buf);
						ori_buf = buf = new_str;
						size = new_strl;
					}
				}
			}
#endif
		}
	}
	if (encoding) free(encoding);
#endif
	auto tok = json_tokener_new();
	if (!tok) {
		console::warn("Can not initialize json parser.");
		free(ori_buf);
		return 1;
	}
	auto root = json_tokener_parse_ex(tok, buf, size);
	free(ori_buf);
	if (!root) {
		console::warn("Can not parse \"%s\" as a JSON file.", fname);
		auto inf = json_tokener_get_error(tok);
		auto err = json_tokener_error_desc(inf);
		if (err) {
			console::info("Detailed error info: %s", err);
		}
		json_tokener_free(tok);
		return 1;
	}
	json_tokener_free(tok);
	if (json_object_get_type(root) != json_type_object) {
		console::warn("The root element of \"%s\" is not a object.\nPlease use a object as a root element.", fname);
		while (!json_object_put(root));
		return 1;
	}
	if (read_json_object<std::string&>(root, "ffplay", &string_callback, conf.ffplay)) {
		console::verbose("Read ffplay setting from \"%s\": %s", fname, conf.ffplay.c_str());
	}
	if (read_json_object<int&>(root, "width", &int_callback, conf.width)) {
		console::verbose("Read width setting from \"%s\": %i", fname, conf.width);
	}
	if (read_json_object<bool&>(root, "autoExit", &bool_callback, conf.autoExit)) {
		console::verbose("Read autoExit settings from \"%s\": %s", fname, conf.autoExit ? "true" : "false");
	}
	while (!json_object_put(root));
	return 0;
}
