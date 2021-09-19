#ifndef _ST_CHARDET_H
#define _ST_CHARDET_H

#include <stddef.h>

namespace chardet {
	/**
	 * @brief Detect encoding
	 * @param buff content buffer
	 * @param len the size of content buffer
	 * @param encoding File encoding, will allocate memory in the function if detect successfully. Need free memory manually.
	 * @param confidence The confidence of result.
	 * @param bom The number of BOM char, if can't detect, set -1
	 * @return true if detect successfully.
	*/
	bool det(const char* buff, size_t buff_len, char*& encoding, float* confidence, short& bom);
}

#endif
