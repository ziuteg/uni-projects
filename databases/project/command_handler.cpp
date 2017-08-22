// Autor: Łukasz Siudek
// Indeks: 283493

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <pqxx/pqxx>
#include <boost/optional.hpp>
#include "libs/json.hpp"
#include "command_handler.h"
#include "db_functions.h"

namespace command_handler {

std::map<std::string, std::string> json_to_map(const json &obj) {
  std::map<std::string, std::string> map;
  for (auto it = obj.begin(); it != obj.end(); it++) {
    std::string str;
    std::stringstream ss;
    ss << it.value();
    ss >> std::quoted(str);
    map[it.key()] = str;
  }
  return map;
}

void extend_json(json &j1, const json &j2) {
    for (const auto &j : json::iterator_wrapper(j2)) {
        j1[j.key()] = j.value();
    }
}

void handle() {
  int cnt = 0;
  db::database conference;

  while (true) {
    cnt++;
    std::string line;
    std::getline(std::cin, line);
    if (line == "quit")
      return;

    try {
      json line_obj = json::parse(line);
      std::string command_name = line_obj.begin().key();
      json args_obj = line_obj.begin().value();
      ArgumentT args = json_to_map(args_obj);

      auto json_result = conference.run(command_name, args);

      if (json_result) {
        json status_obj, data_obj;
        status_obj = STATUS_OK;
        data_obj["data"] = *json_result;
        extend_json(status_obj, data_obj);
        std::cout << status_obj << std::endl;
      }
      else {
        std::cout << STATUS_OK << std::endl;
      }
    }
    catch (const pqxx::sql_error &e) {
      std::cerr << "SQL error: " << e.what() << std::endl;
      std::cerr << "Query was: " << e.query() << std::endl;
      std::cout << STATUS_ERROR << std::endl;
    }
    catch (const std::out_of_range& e) {
      std::cout << STATUS_NOT_IMPLEMENTED << std::endl;
      std::cerr << cnt << ": " << e.what() << std::endl;
    }
    catch (const std::exception &e) {
      std::cout << STATUS_ERROR << std::endl;
      std::cerr << cnt << ": " << e.what() << std::endl;
    }
  }
}
} //command_handler
