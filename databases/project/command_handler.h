// Autor: Łukasz Siudek
// Indeks: 283493

#ifndef __COMMAND_HANDLER__
#define __COMMAND_HANDLER__

#include <string>
#include <map>
#include <functional>
#include <boost/optional.hpp>
#include "libs/json.hpp"
#include "db_functions.h"

namespace command_handler {
  using json = nlohmann::json;
  using HandlerResultT = boost::optional<json>;
  using ArgumentT = std::map<std::string, std::string>;
  using HandlerT = std::function<HandlerResultT (const ArgumentT&)>;

  const json STATUS_OK = {{"status", "OK"}};
  const json STATUS_ERROR = {{"status", "ERROR"}};
  const json STATUS_NOT_IMPLEMENTED = {{"status", "NOT_IMPLEMENTED"}};

  std::map<std::string, std::string> json_to_map(const json &obj);
  void extend_json(json &j1, const json &j2);
  void handle();
} // command_handler
#endif
