#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include "libs/json.hpp"
#include "command_handler.h"


int main() {
  try {
    command_handler::handle();
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}
