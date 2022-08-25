#ifndef DCC_FILE_HPP
#define DCC_FILE_HPP

#include <string>

namespace dcc {

  // Create a directory tree using a given path.
  int create_dirtree(const char* dirtree);

  // Reads an entire file into a given string. Truncates any null bytes.
  // Returns 0 on success, -1 on error;
  int freadall(const char* path, std::string& outstr);

  // Compare two files.
  // Return 0 if they are equal, -1 if error (sets errno) and 1 if the
  // files are different.
  //
  // If given diffpos, will set it to be the position of the first byte
  // where the bytes are different.
  int fcmp(const char* src, const char* dst);
  int fcmp(const char* src, const char* dst, size_t& diffpos);
  int fcmp(const std::string& src, const std::string& dst);
  int fcmp(const std::string& src, const std::string& dst, size_t& diffpos);

  // If the directory structure described by 'dir' does not exist, this
  // function will create it, and then attempt to open the file 'logname'
  // from inside that directory.
  FILE* dirgen_fopen(const char* logname, const char* dir, const char* flags);

}; // namespace dcc

#endif
