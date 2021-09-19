#include "chardet.h"
#include "chardet/chardet.h"
#include <malloc.h>
#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif
#include "console.h"

bool chardet::det(const char* buff, size_t buff_len, char*& encoding, float* confidence, short& bom) {
	auto obj = detect_obj_init();
	if (!obj) {
		console::info("Can not allocate enough memory for detect obj.");
		return false;
	}
	short re = 0;
#ifdef CHARDET_BINARY_SAFE
	re = detect_r(buff, buff_len, &obj);
#else
	if (buff[buff_len - 1] == 0) re = detect(buff, &obj);
	else {
		char* buff2 = (char*)malloc(buff_len + 1);
		if (!buff2) {
			console::warn("Can not allocate enough memory for detect temp buffer, needed size: %zi.", buff_len + 1);
			detect_obj_free(&obj);
			return false;
		}
		memcpy(buff2, buff, buff_len);
		buff2[buff_len] = 0;
		re = detect(buff2, &obj);
		free(buff2);
	}
#endif
	if (re == CHARDET_OUT_OF_MEMORY) {
		console::info("Can not allocate enough memory when detecting file encoding.");
		detect_obj_free(&obj);
		return false;
	}
	size_t len = strlen(obj->encoding);
	encoding = (char*)malloc(len + 1);
	if (!encoding) {
		console::warn("Can not allocate enough memory for detect output encoding, needed size: %zi.", len + 1);
		detect_obj_free(&obj);
		return false;
	}
#ifdef HAVE_STRCPY_S
	if (strcpy_s(encoding, len + 1, obj->encoding)) {
		console::error("Can not copy detect encoding to output encoding when detecting.\nThis should be a bug");
		free(encoding);
		encoding = nullptr;
		detect_obj_free(&obj);
		return false;
	}
#else
	strcpy(encoding, obj->encoding);
#endif
	if (confidence) *confidence = obj->confidence;
#ifdef CHARDET_BOM_CHECK
	bom = obj->bom;
#else
	bom = -1;
#endif
	detect_obj_free(&obj);
	return true;
}
