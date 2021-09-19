#ifndef _ST_JSONC_H
#define _ST_JSONC_H

#include "configfile.h"

namespace jsonc {
	int read_json_file(const char* fname, config& conf);
}

#endif
