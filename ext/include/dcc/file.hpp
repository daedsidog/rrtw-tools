#ifndef DCC_FILE_HPP
#define DCC_FILE_HPP

#include <string>

namespace dcc {

  int create_dir_tree(const char* dirtree = nullptr);
  int freadall(const char* path, std::string& outstr);

  // If the directory structure described by 'dir' does not exist, this
  // function will create it, and then attempt to open the file 'logname'
  // from inside that directory.
  FILE* dirgen_fopen(const char* logname, const char* dir, const char* flags);

}; // namespace dcc

#endif
