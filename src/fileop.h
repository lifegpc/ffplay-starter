#ifndef _ST_FILEOP_H
#define _ST_FILEOP_H
#include <stdio.h>
#include <string>
#include <list>

#define DefaultMaxFileNameSize (512 * 1024)

namespace fileop {
	/**
	 * \brief Open file.
	 * \param fname File Name (UTF-8 / ANSI encoding)
	 * \param mode open mode, such as rb
	 * \return the pointer to FILE.
	 */
	FILE* open(const char *fname, const char *mode);
	/**
	 * \brief Check file exists
	 * \param fname File Name
	 * \param use_wchar whether to convert to WCHAR first on win32 platform
	 * \return Whether file exists or not
	*/
	bool exists(const char* fname, bool use_wchar = true);
	/**
	 * \brief Check file exists
	 * \param fname File Name
	 * \param use_wchar whether to convert to WCHAR first on win32 platform
	 * \return Whether file exists or not
	*/
	bool exists(std::string fname, bool use_wchar = true);
	/**
	 * \brief Check file exists
	 * \param fname File Name
	 * \return Whether file exists or not
	*/
	bool exists(const wchar_t* fname);
	/**
	 * \brief Close file
	 * \param f Pointer to FILE structure
	 * \return true if closed successfully
	*/
	bool close(FILE* f);
	/**
	 * @brief Split file name to name without ext and ext.
	 * @param fname File name
	 * @param name Name without ext. Can be NULL if don't needed.
	 * @param ext Ext (contains `.`). Can be NULL if don't needed.
	 * @return true if OK
	*/
	bool splitext(const std::string fname, std::string* name, std::string* ext);
	/**
	 * \brief Concat file path. гиJust simple base + ext).
	 * This function will alloc memory by calling malloc, need free mannally.
	 * \param base Base
	 * \param ext Ext
	 * \param path Result
	 * \return true if OK
	*/
	bool concat(const char* base, const char* ext, char*& path);
	/**
	 * \brief Get file size
	 * \param f The pointer to FILE structure.
	 * \param size File size
	 * \return true if OK
	*/
	bool filesize(FILE* f, size_t& size);
	/**
	 * \brief Get the current position
	 * \param f the pointer of FILE structure.
	 * \param loc Location
	 * \return true if OK
	*/
	bool tell(FILE* f, size_t& loc);
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Convert input string to UTF-8 encoding
	 * @param input Input file name
	 * @param output Output file name (Need free memory)
	 * @return true if OK
	*/
	bool convertToUTF8(const char* input, char*& output);
	/**
	 * @brief Get Open File Name by Popup a dialog
	 * @param filename File Name (Need free memory)
	 * @param filter Filter string
	 * @param title Dialog title
	 * @param initialDir Inital Dir
	 * @param default_flags whether to use default flags
	 * @param flags custom flags
	 * @param maxFileNameSize The max size of filename.
	 * @return true if OK
	*/
	bool getOpenFileName(char*& filename, const wchar_t* filter = nullptr, const wchar_t* title = nullptr, const wchar_t* initialDir = nullptr, bool default_flags = true, unsigned long flags = 0, unsigned long maxFileNameSize = DefaultMaxFileNameSize);
#endif
	/**
	 * @brief Creates a pipe and executes a command
	 * @param Command to be executed.
	 * @param mode Mode of the returned stream.
	 * @return a stream associated with one end of the created pipe.
	*/
	FILE* popen(const char* command, const char* mode);
	/**
	 * @brief close a pipe stream to or from a process
	 * @param f pipe
	 * @return Upon successful return, pclose() shall return the termination status of the command language interpreter. Otherwise, pclose() shall return -1 and set errno to indicate the error.
	*/
	int pclose(FILE* f);
	/**
	 * @brief Executes a command
	 * @param command The command to be executed.
	 * @return command return value
	*/
	int system(const char* command);
	/**
	 * @brief Get program location
	 * @return the program's location, if can not find, will be empty
	*/
	std::string getProgramLocation();
#if defined(_WIN32) && !defined(__CYGWIN__)
	/**
	 * @brief Searches a directory for a file or subdirectory with a name that matches a specific name (or partial name if wildcards are used)
	 * @param fname The directory or path, and the file name.
	 * @param fl Result list
	 * @return true if OK
	*/
	bool win32FindFile(const char* fname, std::list<std::string>& fl);
#endif
	/**
	 * @brief Combine Directory and File Name
	 * @param base Directory name
	 * @param name File Name
	 * @return Result
	*/
	std::string combilePath(std::string base, std::string name);
	/**
	 * @brief split path to directory name and file name
	 * @param path The path
	 * @param dir The directory name
	 * @param name The file name
	 * @return true if OK
	*/
	bool split(std::string path, std::string* dir, std::string* name);
	/**
	 * @brief Add Directory to File Name list
	 * @param base Directory
	 * @param fl File Name list
	 * @return true if OK
	*/
	bool addDirectoryToFileNameList(std::string base, std::list<std::string>& fl);
	/**
	 * @brief List directory
	 * @param path directory path
	 * @param fl File list
	 * @param fullpath Whether to return full path
	 * @return true if OK
	*/
	bool listdir(std::string path, std::list<std::string>& fl, bool fullpath = true);
	/**
	 * @brief List relative files of file
	 * @param file The file path
	 * @param fl The list of relative files (Full path)
	 * @return true if OK
	*/
	bool listrelative(std::string file, std::list<std::string>& fl);
	/**
	 * @brief Filter file list by 
	 * @param fl File list (Full path)
	 * @param exts Ext list
	 * @param result Filtered file list (Full path)
	 * @param filter_no_ext whether to filter file which don't have a ext
	 * @return true if OK
	*/
	bool filterFileListByExt(std::list<std::string> fl, std::list<std::string> exts, std::list<std::string>& result, bool filter_no_ext = true);
}

#endif
