/**
 * Łukasz Siudek
 * 283493
 **/

#include <iostream>
#include <cstring>
#include <string>

#include "server.h"

#define ARGUMENT_COUNT 3

namespace {
  int port;
  std::string directory;
}

void usage(std::string name) {
  std::cerr << "Usage: " << name << " <port> <directory>" << std::endl;
}

void parse_input(int argc, char **argv) {
  if (argc != ARGUMENT_COUNT) {
    usage(argv[0]);
    throw std::runtime_error("Error: invalid input");
  }
  port = std::stoi(argv[1]);
  directory = std::string(argv[2]);
}

int main(int argc, char **argv) {
  try {
    parse_input(argc, argv);
    server(port, directory);
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unexpected exception" << std::endl;
    throw;
  }
  return 0;
}
