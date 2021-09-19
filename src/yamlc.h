#ifndef _ST_YAMLC_H
#define _ST_YAMLC_H

#include "configfile.h"

namespace yamlc {
	int read_yaml_file(const char* fname, config& conf);
}

#endif
