#include <sys/stat.h>
#include <string>
#include <fstream>
#include <limits>

bool is_directory(std::string path) {
  struct stat buffer;
  stat(path.c_str(), &buffer);
  return S_ISDIR(buffer.st_mode);
}

bool is_file(std::string path) {
  struct stat buffer;
  stat(path.c_str(), &buffer);
  return S_ISREG(buffer.st_mode);
}

std::string get_extension(std::string path) {
  int index = path.find_last_of(".");
  return path.substr(index + 1);
}

std::streamsize file_size(std::ifstream &file) {
  std::streampos pos = file.tellg();
  file.ignore(std::numeric_limits<std::streamsize>::max());
  std::streamsize length = file.gcount();
  file.clear();
  file.seekg(pos, std::ios_base::beg);
  return length;
}
