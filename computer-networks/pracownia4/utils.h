#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/stat.h>
#include <string>

bool is_directory(std::string path);
bool is_file(std::string path);
std::string get_extension(std::string path);
std::streamsize file_size(std::ifstream &file);

#endif
