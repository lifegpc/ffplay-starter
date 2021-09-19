#ifdef HAVE_ST_CONFIG_H
#include "config.h"
#endif
#include "chariconv.h"
#include "console.h"
#include <malloc.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_ICONV
#include "iconv.h"
#endif
#include <ctype.h>
#ifndef HAVE_PCRE
#include <string>
#include <regex>
#endif
#include <stdio.h>
#include <stdarg.h>
#include "util.h"

#ifdef HAVE_SSCANF_S
#define sscanf sscanf_s
#endif

#ifdef HAVE_ICONV
bool iconv_convert(const char* input, size_t input_size, char*& output, size_t& output_size, const char* ori_enc, const char* des_enc) {
	console::verbose("Try use iconv to convert from '%s' to '%s'.", ori_enc, des_enc);
	iconv_t cd = iconv_open(des_enc, ori_enc);
	if (cd == (iconv_t)-1) {
		if (errno == EINVAL) {
			console::info("Iconv: Conversion from '%s' to '%s' not available.", ori_enc, des_enc);
		} else {
			console::verbose("An error occured when calling iconv_open.");
		}
		return false;
	}
	char* first = (char*)malloc(input_size);
	char* now = first;
	char* now_in = (char*)input;
	size_t all = input_size;
	size_t avail_out = input_size, avail_in = input_size;
	if (!first) {
		console::warn("Can not allocate memory, needed size: %zi.", input_size);
		if (iconv_close(cd)) console::verbose("An error occured when closing iconv.");
		return false;
	}
	while (avail_in >= 0) {
		if (avail_out < input_size) {
			size_t needed = all + input_size;
			char* newstr = (char*)realloc(first, needed);
			if (!newstr) {
				console::warn("Can not reallocate memory, needed size: %zi.", needed);
				free(first);
				if (iconv_close(cd)) console::verbose("An error occured when closing iconv.");
				return false;
			}
			if (newstr != first) {
				now = newstr + (all - avail_out);
			}
			first = newstr;
			all += input_size;
			avail_out += input_size;
		}
		if (avail_in > 0) {
			if (iconv(cd, &now_in, &avail_in, &now, &avail_out) == -1) {
				console::verbose("An error occured when converting from '%s' to '%s'.", ori_enc, des_enc);
				free(first);
				if (iconv_close(cd)) console::verbose("An error occured when closing iconv.");
				return false;
			}
		} else {
			*now = 0;
			now++;
			avail_out--;
			break;
		}
	}
	output = first;
	output_size = all - avail_out - 1;
	char* news = (char*)realloc(output, output_size + 1);
	if (news) output = news;
	console::verbose("Convert from '%s' to '%s' successfully. (%zi bytes -> %zi bytes)", ori_enc, des_enc, input_size, output_size);
	return true;
}
#endif

bool chariconv::convert(const char* input, size_t input_size, char*& output, size_t& output_size, const char* ori_enc, const char* des_enc, bool iconv_only) {
	if (!input || !ori_enc || !des_enc) return false;
	bool re = false;
#ifdef HAVE_ICONV
	re = iconv_convert(input, input_size, output, output_size, ori_enc, des_enc);
	if (re || iconv_only) return re;
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
	UINT ori_cp, des_cp;
	if (encodingToCp(ori_enc, ori_cp) && encodingToCp(des_enc, des_cp)) {
		return convert(input, input_size, output, output_size, ori_cp, des_cp);
	}
#endif
	return re;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
bool chariconv::convert(const char* input, size_t input_size, char*& output, size_t& output_size, const UINT ori_cp, const UINT des_cp) {
	if (!input)return false;
	console::verbose("Convert from cp%u (%s) to cp%u (%s).", ori_cp, cpToEncoding(ori_cp), des_cp, cpToEncoding(des_cp));
	wchar_t* ws;
	int wlen = 0;
	DWORD opt = util::getMultiByteToWideCharOptions(MB_ERR_INVALID_CHARS, ori_cp);
	wlen = MultiByteToWideChar(ori_cp, opt, input, input_size, nullptr, 0);
	if (!wlen) {
		console::verbose("Can not convert string from Code Page %u by using MultiByteToWideChar.", ori_cp);
		return false;
	}
	ws = (wchar_t*)malloc(sizeof(wchar_t) * wlen);
	if (!ws) {
		console::warn("Can not reallocate memory, needed size: %zi.", sizeof(wchar_t) * wlen);
		return false;
	}
	if (!MultiByteToWideChar(ori_cp, opt, input, input_size, ws, wlen)) {
		free(ws);
		console::verbose("Can not convert string from Code Page %u by using MultiByteToWideChar.", ori_cp);
		return false;
	}
	char* ns;
	DWORD opt2 = util::getWideCharToMultiByteOptions(WC_ERR_INVALID_CHARS, des_cp);
	int nlen;
	nlen = WideCharToMultiByte(des_cp, opt2, ws, wlen, nullptr, 0, nullptr, FALSE);
	if (!nlen) {
		free(ws);
		console::verbose("Can not convert wstring to Code Page %u by using WideCharToMultiByte.", des_cp);
		return false;
	}
	ns = (char*)malloc(nlen + 1);
	if (!ns) {
		free(ws);
		console::warn("Can not reallocate memory, needed size: %zi.", nlen);
		return false;
	}
	if (!WideCharToMultiByte(des_cp, opt2, ws, wlen, ns, nlen, nullptr, FALSE)) {
		free(ws);
		free(ns);
		console::verbose("Can not convert wstring to Code Page %u by using WideCharToMultiByte.", des_cp);
		return false;
	}
	free(ws);
	ns[nlen] = 0;
	output = ns;
	output_size = nlen;
	console::verbose("Convert from 'cp%u (%s)' to 'cp%u (%s)' successfully. (%zi bytes -> %zi bytes)", ori_cp, cpToEncoding(ori_cp), des_cp, cpToEncoding(des_cp), input_size, output_size);
	return true;
}

bool chariconv::convert(const wchar_t* input, int input_size, char*& output, size_t& output_size, const UINT des_cp) {
	if (!input) return false;
	console::verbose("Convert from wstring to cp%u (%s).", des_cp, cpToEncoding(des_cp));
	if (input_size <= 0) input_size = lstrlenW(input);
	char* ns;
	DWORD opt = util::getWideCharToMultiByteOptions(WC_ERR_INVALID_CHARS, des_cp);
	int nlen;
	nlen = WideCharToMultiByte(des_cp, opt, input, input_size, nullptr, 0, nullptr, FALSE);
	if (!nlen) {
		console::verbose("Can not convert wstring to Code Page %u by using WideCharToMultiByte.", des_cp);
		return false;
	}
	ns = (char*)malloc(nlen + 1);
	if (!ns) {
		console::warn("Can not reallocate memory, needed size: %zi.", nlen);
		return false;
	}
	if (!WideCharToMultiByte(des_cp, opt, input, input_size, ns, nlen, nullptr, FALSE)) {
		free(ns);
		console::verbose("Can not convert wstring to Code Page %u by using WideCharToMultiByte.", des_cp);
		return false;
	}
	ns[nlen] = 0;
	output = ns;
	output_size = nlen;
	console::verbose("Convert from wstring to 'cp%u (%s)' successfully. (%zi bytes -> %zi bytes)", des_cp, cpToEncoding(des_cp), input_size * sizeof(wchar_t), output_size);
	return true;
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
const char* chariconv::cpToEncoding(UINT cp) {
	switch (cp)
	{
	case CP_ACP:
		return cpToEncoding(GetACP());
	case CP_OEMCP:
		return cpToEncoding(GetOEMCP());
	case 37:
		return "ibm037";
	case 437:
		return "ibm437";
	case 500:
		return "ibm500";
	case 708:
		return "asmo-708";
	case 720:
		return "dos-720";
	case 737:
		return "ibm737";
	case 775:
		return "ibm775";
	case 850:
		return "ibm850";
	case 852:
		return "ibm852";
	case 855:
		return "ibm855";
	case 857:
		return "ibm857";
	case 858:
		return "ibm00858";
	case 860:
		return "ibm860";
	case 861:
		return "ibm861";
	case 862:
		return "dos-862";
	case 863:
		return "ibm863";
	case 864:
		return "ibm864";
	case 865:
		return "ibm865";
	case 866:
		return "cp866";
	case 869:
		return "ibm869";
	case 870:
		return "ibm870";
	case 874:
		return "windows-874";
	case 875:
		return "cp875";
	case 932:
		return "shift_jis";
	case 936:
		return "gb2312";
	case 949:
		return "ks_c_5601-1987";
	case 950:
		return "big5";
	case 1026:
		return "ibm1026";
	case 1047:
		return "ibm01047";
	case 1140:
		return "ibm01140";
	case 1141:
		return "ibm01141";
	case 1142:
		return "ibm01142";
	case 1143:
		return "ibm01143";
	case 1144:
		return "ibm01144";
	case 1145:
		return "ibm01145";
	case 1146:
		return "ibm01146";
	case 1147:
		return "ibm01147";
	case 1148:
		return "ibm01148";
	case 1149:
		return "ibm01149";
	case 1200:
		return "utf-16le";
	case 1201:
		return "utf-16be";
	case 1250:
		return "windows-1250";
	case 1251:
		return "windows-1251";
	case 1252:
		return "windows-1252";
	case 1253:
		return "windows-1253";
	case 1254:
		return "windows-1254";
	case 1255:
		return "windows-1255";
	case 1256:
		return "windows-1256";
	case 1257:
		return "windows-1257";
	case 1258:
		return "windows-1258";
	case 1361:
		return "johab";
	case 10000:
		return "macintosh";
	case 10001:
		return "x-mac-japanese";
	case 10002:
		return "x-mac-chinesetrad";
	case 10003:
		return "x-mac-korean";
	case 10004:
		return "x-mac-arabic";
	case 10005:
		return "x-mac-hebrew";
	case 10006:
		return "x-mac-greek";
	case 10007:
		return "x-mac-cyrillic";
	case 10008:
		return "x-mac-chinesesimp";
	case 10010:
		return "x-mac-romanian";
	case 10017:
		return "x-mac-ukrainian";
	case 10021:
		return "x-mac-thai";
	case 10029:
		return "x-mac-ce";
	case 10079:
		return "x-mac-icelandic";
	case 10081:
		return "x-mac-turkish";
	case 10082:
		return "x-mac-croatian";
	case 12000:
		return "utf-32le";
	case 12001:
		return "utf-32be";
	case 20000:
		return "x-chinese_cns";
	case 20001:
		return "x-cp20001";
	case 20002:
		return "x_chinese-eten";
	case 20003:
		return "x-cp20003";
	case 20004:
		return "x-cp20004";
	case 20005:
		return "x-cp20005";
	case 20105:
		return "x-ia5";
	case 20106:
		return "x-ia5-german";
	case 20107:
		return "x-ia5-swedish";
	case 20108:
		return "x-ia5-norwegian";
	case 20127:
		return "us-ascii";
	case 20261:
		return "x-cp20261";
	case 20269:
		return "x-cp20269";
	case 20273:
		return "ibm273";
	case 20277:
		return "ibm277";
	case 20278:
		return "ibm278";
	case 20280:
		return "ibm280";
	case 20284:
		return "ibm284";
	case 20285:
		return "ibm285";
	case 20290:
		return "ibm290";
	case 20297:
		return "ibm297";
	case 20420:
		return "ibm420";
	case 20423:
		return "ibm423";
	case 20424:
		return "ibm424";
	case 20833:
		return "x-ebcdic-koreanextended";
	case 20838:
		return "ibm-thai";
	case 20866:
		return "koi8-r";
	case 20871:
		return "ibm871";
	case 20880:
		return "ibm880";
	case 20905:
		return "ibm295";
	case 20924:
		return "ibm00924";
	case 20932:
	case 51932:
		return "euc-jp";
	case 20936:
		return "x-cp20936";
	case 20949:
		return "x-cp20949";
	case 21025:
		return "cp1025";
	case 21866:
		return "koi8-u";
	case 28591:
		return "iso-8859-1";
	case 28592:
		return "iso-8859-2";
	case 28593:
		return "iso-8859-3";
	case 28594:
		return "iso-8859-4";
	case 28595:
		return "iso-8859-5";
	case 28596:
		return "iso-8859-6";
	case 28597:
		return "iso-8859-7";
	case 28598:
		return "iso-8859-8";
	case 28599:
		return "iso-8859-9";
	case 28603:
		return "iso-8859-13";
	case 28605:
		return "iso-8859-15";
	case 29001:
		return "x-europa";
	case 38598:
		return "iso-8859-8-i";
	case 50220:
	case 50222:
		return "iso-2022-jp";
	case 50221:
		return "csiso2022jp";
	case 50225:
		return "iso-2022-kr";
	case 50227:
		return "x-cp50227";
	case 51936:
		return "euc-cn";
	case 51949:
		return "euc-kr";
	case 52936:
		return "hz-gb-2312";
	case 54936:
		return "gb18030";
	case 57002:
		return "x-iscii-de";
	case 57003:
		return "x-iscii-be";
	case 57004:
		return "x-iscii-ta";
	case 57005:
		return "x-iscii-te";
	case 57006:
		return "x-iscii-as";
	case 57007:
		return "x-iscii-or";
	case 57008:
		return "x-iscii-ka";
	case 57009:
		return "x-iscii-ma";
	case 57010:
		return "x-iscii-gu";
	case 57011:
		return "x-iscii-pa";
	case CP_UTF7:
		return "utf-7";
	case CP_UTF8:
		return "utf-8";
	default:
		return nullptr;
	}
}
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
bool chariconv::encodingToCp(const char* encoding, UINT& cp) {
	if (!encoding) return false;
	char* s = nullptr;
	if (!tolowercase(encoding, 0, s)) return false;
#define ref(x) return free(s), cp = (x), true
#ifdef HAVE_PCRE
	size_t slen = strlen(s);
	static pcre* reg = nullptr;
	if (!reg) util::comppcre(R"(^cp(\d+)$)", reg);
	if (reg) {
		int vect[6];
		auto re = pcre_exec(reg, nullptr, s, slen, 0, 0, vect, 6);
		if (re > -1) {
			UINT tcp;
			if (util::extractUInt(s, vect[2], vect[3], tcp)) {
#else
	std::string ss(s);
	static const std::regex reg(R"(^cp(\d+)$)");
	std::smatch re;
	if (std::regex_search(ss, re, reg)) {
		auto res = re[1];
		auto ts = res.str();
		auto cs = ts.c_str();
		if (cs) {
			UINT tcp;
			if (sscanf(cs, "%u", &tcp) > 0) {
#endif
				switch (tcp)
				{
				case 1025:
					ref(21025U);
				default:
					ref(tcp);
				}
			}
		}
	}
#ifdef HAVE_PCRE
	static pcre* reg2 = nullptr;
	if (!reg2) util::comppcre(R"(^x-cp(\d+)$)", reg2);
	if (reg2) {
		int vect[6];
		auto re = pcre_exec(reg2, nullptr, s, slen, 0, 0, vect, 6);
		if (re > -1) {
			UINT tcp;
			if (util::extractUInt(s, vect[2], vect[3], tcp)) {
#else
	static const std::regex reg2(R"(^x-cp(\d+)$)");
	if (std::regex_match(ss, re, reg2)) {
		auto res = re[1];
		auto ts = res.str();
		auto cs = ts.c_str();
		if (cs) {
			UINT tcp;
			if (sscanf(cs, "%u", &tcp) > 0) {
#endif
				ref(tcp);
			}
		}
	}
#ifdef HAVE_PCRE
	static pcre* reg3 = nullptr;
	if (!reg3) util::comppcre(R"(^ibm(\d+)$)", reg3);
	if (reg3) {
		int vect[6];
		auto re = pcre_exec(reg3, nullptr, s, slen, 0, 0, vect, 6);
		if (re > -1) {
			UINT tcp;
			if (util::extractUInt(s, vect[2], vect[3], tcp)) {
#else
	static const std::regex reg3(R"(^ibm(\d+)$)");
	if (std::regex_match(ss, re, reg3)) {
		auto res = re[1];
		auto ts = res.str();
		auto cs = ts.c_str();
		if (cs) {
			UINT tcp;
			if (sscanf(cs, "%u", &tcp) > 0) {
#endif
				switch (tcp)
				{
				case 273:
				case 277:
				case 278:
				case 280:
				case 284:
				case 285:
				case 290:
				case 297:
				case 420:
				case 423:
				case 424:
				case 871:
				case 880:
				case 905:
				case 924:
					ref(tcp + 20000U);
				default:
					ref(tcp);
				}
			}
		}
	}
#ifdef HAVE_PCRE
	static pcre* reg4 = nullptr;
	if (!reg4) util::comppcre(R"(^windows-(\d+)$)", reg4);
	if (reg4) {
		int vect[6];
		auto re = pcre_exec(reg4, nullptr, s, slen, 0, 0, vect, 6);
		if (re > -1) {
			UINT tcp;
			if (util::extractUInt(s, vect[2], vect[3], tcp)) {
#else
	static const std::regex reg4(R"(^windows-(\d+)$)");
	if (std::regex_match(ss, re, reg4)) {
		auto res = re[1];
		auto ts = res.str();
		auto cs = ts.c_str();
		if (cs) {
			UINT tcp;
			if (sscanf(cs, "%u", &tcp) > 0) {
#endif
				ref(tcp);
			}
		}
	}
#ifdef HAVE_PCRE
	static pcre* reg5 = nullptr;
	if (!reg5) util::comppcre(R"(^iso-8859-(\d+)$)", reg5);
	if (reg5) {
		int vect[6];
		auto re = pcre_exec(reg5, nullptr, s, slen, 0, 0, vect, 6);
		if (re > -1) {
			UINT tcp;
			if (util::extractUInt(s, vect[2], vect[3], tcp)) {
#else
	static const std::regex reg5(R"(^iso-8859-(\d+)$)");
	if (std::regex_match(ss, re, reg5)) {
		auto res = re[1];
		auto ts = res.str();
		auto cs = ts.c_str();
		if (cs) {
			UINT tcp;
			if (sscanf(cs, "%u", &tcp) > 0) {
#endif
				ref(tcp + 28590U);
			}
		}
	}
	if (!strcmp(s, "asmo-708")) ref(708U);
	if (!strcmp(s, "dos-720")) ref(720U);
	if (!strcmp(s, "dos-862")) ref(862U);
	if (!strncmp(s, "gb2312", 6)) ref(936U);
	if (!strcmp(s, "ks_c_5601-1987")) ref(949U);
	if (!strncmp(s, "big5", 4)) ref(950U);
	if (strcmpEx(s, "utf16", 3, "utf-16", "utf-16le", "utf16le")) ref(1200U);
	if (strcmpEx(s, "unicodefffe", 2, "utf-16be", "utf16be")) ref(1201U);
	if (!strcmp(s, "johab")) ref(1361U);
	if (strcmpEx(s, "macintosh", 1, "macroman")) ref(10000U);
	if (!strcmp(s, "x-mac-japanese")) ref(10001U);
	if (!strcmp(s, "x-mac-chinesetrad")) ref(10002U);
	if (!strcmp(s, "x-mac-korean")) ref(10003U);
	if (strcmpEx(s, "x-mac-arabic", 1, "macarabic")) ref(10004U);
	if (strcmpEx(s, "x-mac-hebrew", 1, "machebrew")) ref(10005U);
	if (strcmpEx(s, "x-mac-greek", 1, "macgreek")) ref(10006U);
	if (strcmpEx(s, "x-mac-cyrillic", 1, "maccyrillic")) ref(10007U);
	if (!strcmp(s, "x-mac-chinesesimp")) ref(10008U);
	if (strcmpEx(s, "x-mac-romanian", 1, "macromania")) ref(10010U);
	if (strcmpEx(s, "x-mac-ukrainian", 1, "macukraine")) ref(10017U);
	if (strcmpEx(s, "x-mac-thai", 1, "macthai")) ref(10021U);
	if (!strcmp(s, "x-mac-ce")) ref(10029U);
	if (strcmpEx(s, "x-mac-icelandic", 1, "maciceland")) ref(10079U);
	if (strcmpEx(s, "x-mac-turkish", 1, "macturkish")) ref(10081U);
	if (strcmpEx(s, "x-mac-croatian", 1, "maccroatian")) ref(10082U);
	if (strcmpEx(s, "utf32", 3, "utf-32", "utf-32le", "utf32le")) ref(12000U);
	if (strcmpEx(s, "utf-32be", 1, "utf32be")) ref(12001U);
	if (!strcmp(s, "x-chinese_cns")) ref(20000U);
	if (!strcmp(s, "x_chinese-eten")) ref(20002U);
	if (!strcmp(s, "x-ia5")) ref(20105U);
	if (!strcmp(s, "x-ia5-german")) ref(20106U);
	if (!strcmp(s, "x-ia5-swedish")) ref(20107U);
	if (!strcmp(s, "x-ia5-norwegian")) ref(20108U);
	if (strcmpEx(s, "ascii", 1, "us-ascii")) ref(20127U);
	if (!strcmp(s, "x-ebcdic-koreanextended")) ref(20833U);
	if (!strcmp(s, "ibm-thai")) ref(20838U);
	if (!strcmp(s, "koi8-r")) ref(20866U);
	if (!strcmp(s, "euc-jp")) ref(20932U);
	if (!strcmp(s, "koi8-u")) ref(21866U);
	if (!strcmp(s, "x-europa")) ref(29001U);
	if (!strcmp(s, "iso-8859-8-i")) ref(38598U);
	if (!strcmp(s, "iso-2022-jp")) ref(50222U);
	if (!strcmp(s, "csiso2022jp")) ref(50221U);
	if (!strcmp(s, "iso-2022-kr")) ref(50225U);
	if (!strcmp(s, "euc-cn")) ref(51936U);
	if (!strcmp(s, "euc-kr")) ref(51949U);
	if (!strcmp(s, "hz-gb-2312")) ref(52936U);
	if (!strcmp(s, "gb18030")) ref(54936U);
	if (!strcmp(s, "x-iscii-de")) ref(57002U);
	if (!strcmp(s, "x-iscii-be")) ref(57003U);
	if (!strcmp(s, "x-iscii-ta")) ref(57004U);
	if (!strcmp(s, "x-iscii-te")) ref(57005U);
	if (!strcmp(s, "x-iscii-as")) ref(57006U);
	if (!strcmp(s, "x-iscii-or")) ref(57007U);
	if (!strcmp(s, "x-iscii-ka")) ref(57008U);
	if (!strcmp(s, "x-iscii-ma")) ref(57009U);
	if (!strcmp(s, "x-iscii-gu")) ref(57010U);
	if (!strcmp(s, "x-iscii-pa")) ref(57011U);
	if (strcmpEx(s, "utf-7", 1, "utf7")) ref(CP_UTF7);
	if (strcmpEx(s, "utf-8", 1, "utf8")) ref(CP_UTF8);
#undef ref
	free(s);
	return false;
}
#endif

bool chariconv::strcmpEx(const char* s, const char* c, int count, ...) {
	if (!strcmp(s, c)) return true;
	va_list li;
	va_start(li, count);
	while (count > 0) {
		auto t = va_arg(li, const char*);
		if (!strcmp(s, t)) return t;
		count--;
	}
	va_end(li);
	return false;
}

bool chariconv::tolowercase(const char* input, size_t input_size, char*& output) {
	if (!input) return false;
	if (!input_size) input_size = strlen(input);
	if (!input_size) return false;
	output = (char*)malloc(input_size + 1);
	if (!output) {
		console::warn("Can not reallocate memory, needed size: %zi.", input_size + 1);
		return false;
	}
	size_t i = 0;
	for (; i < input_size; i++) {
		output[i] = tolower(input[i]);
	}
	output[input_size] = 0;
	return true;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
bool chariconv::WStringToUTF8(std::wstring source, std::string& dest) {
	if (source.empty()) {
		dest = "";
		return true;
	}
	char* s;
	size_t slen;
	if (!convert(source.c_str(), source.size(), s, slen, CP_UTF8)) return false;
	dest = s;
	free(s);
	return true;
}
#endif
