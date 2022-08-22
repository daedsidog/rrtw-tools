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
  // Return 0 if they are equal, -1 if error (sets errno) and the position
  // of the first byte where the files differ if find_diffbyte is set to
  // true, and 1 otherwise.
  int fcmp(const char* src, const char* dst, bool find_diffbyte = false);
  int fcmp(const std::string& src, const std::string& dst,
           bool find_diffbyte = false);

  // If the directory structure described by 'dir' does not exist, this
  // function will create it, and then attempt to open the file 'logname'
  // from inside that directory.
  FILE* dirgen_fopen(const char* logname, const char* dir, const char* flags);

}; // namespace dcc

#endif
