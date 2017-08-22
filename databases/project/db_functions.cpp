// Autor: Łukasz Siudek
// Indeks: 283493

#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <functional>
#include <boost/optional.hpp>
#include <pqxx/pqxx>
#include "libs/va.hpp"
#include "libs/json.hpp"
#include "db_functions.h"

namespace db {
  using json = nlohmann::json;
  using HandlerResultT = boost::optional<json>;
  using ArgumentT = std::map<std::string, std::string>;
  using HandlerT = std::function<HandlerResultT (const ArgumentT&)>;

  const std::string secret_str =  "d8578edf8458ce06fbc5bb76a58c5ca4";
  std::unique_ptr<pqxx::connection> C;

  HandlerResultT result_to_json(const pqxx::result &R, int limit) {
    auto objects = json::array();
    bool all = (limit == 0);
    for (auto row : R) {
      json obj;
      if (limit == 0 && !all)
        break;
      for (auto field : row) {
        obj[field.name()] = field.c_str();
      }
      objects.push_back(obj);
      limit--;
    }
    return objects;
  }

  void database::verify_authorization(const Function &function, const ArgumentT &args) {
    if (function.auth_level != UserStatus::USER)
      if (function.auth_level != UserStatus::ORGANIZER)
        return;

    std::string login;
    if (args.count("login")) login = args.at("login");
    else login = args.at("login1");

    auto status = get_user_status(login, args.at("password"));
    if (status == UserStatus::NONE)
      throw std::runtime_error("User doesn't exist");
    if (function.auth_level != status)
      throw std::runtime_error("Unauthorized user");
  }

  HandlerResultT database::run(const std::string &command_name, const ArgumentT &args) {
    const auto &function = functions.at(command_name);

    if (command_name != "open" and !C)
      throw std::runtime_error("Database must be opened before use");

    verify_authorization(function, args);
    return function.handler(args);
  }

  void database::prepare() {
    for (auto f : functions) {
      auto name = f.first;
      auto sql = f.second.sql;
      (*C).prepare(name, sql);
    }
  }

  UserStatus database::get_user_status(const std::string &login, const std::string &password) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("find_user")(login)(password).exec();
    if (R.empty())
      return UserStatus::NONE;
    std::string status = R[0]["status"].c_str();
    if (status == "user")
      return UserStatus::USER;
    if (status == "organizer")
      return UserStatus::ORGANIZER;
    return UserStatus::NONE;
  }

  TalkStatus database::get_talk_status(const std::string &talk) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("find_talk")(talk).exec();
    if (R.empty())
      return TalkStatus::NONE;
    std::string status = R[0]["status"].c_str();
    if (status == "proposed")
      return TalkStatus::PROPOSED;
    if (status == "accepted")
      return TalkStatus::ACCEPTED;
    if (status == "rejected")
      return TalkStatus::REJECTED;
    return TalkStatus::NONE;
  }

  bool database::find_event(const std::string &eventname) {
    if (eventname == "")
      return true;
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("find_event")(eventname).exec();
    if (R.empty())
      return false;
    return true;
  }

  HandlerResultT database::open(const ArgumentT &args) {
    std::string parameters = va("dbname = %1% user = %2% password = %3%",
      args.at("baza"),
      args.at("login"),
      args.at("password")
    );

    C = std::make_unique<pqxx::connection>(parameters);
    if (!C->is_open())
    throw std::runtime_error("Can't open database");

    std::ifstream file;
    std::stringstream buffer;
    std::string sql;
    pqxx::work W(*C);
    file.open("physical_model.sql", std::ios::in);
    if (!file.is_open())
      throw std::runtime_error("Can't open physical model file");
    buffer << file.rdbuf();
    sql = buffer.str();

    try {
      W.exec(sql);
      W.commit();
    }
    catch (const pqxx::sql_error &e) {
      std::cerr << "Physical model is bad or already in use" << std::endl;
    }

    prepare();
    return {};
  }

  HandlerResultT database::organizer(const ArgumentT &args) {
    if (args.at("secret") == secret_str) {
      pqxx::work W(*C);
      W.prepared("organizer")
        (args.at("newlogin"))
        (args.at("newpassword")).exec();
      W.commit();
    }
    else throw std::invalid_argument("Wrong secret argument");
    return {};
  }

  HandlerResultT database::event(const ArgumentT &args) {
    pqxx::work W(*C);
    W.prepared("event")
      (args.at("start_timestamp"))
      (args.at("end_timestamp"))
      (args.at("eventname")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::user(const ArgumentT &args) {
    pqxx::work W(*C);
    W.prepared("user")
      (args.at("newlogin"))
      (args.at("newpassword")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::talk(const ArgumentT &args) {
    if (!find_event(args.at("eventname")))
      throw std::runtime_error("Event doesn't exist");
    pqxx::work W(*C);
    W.prepared("talk")
      (args.at("talk"))
      (args.at("title"))
      (args.at("start_timestamp"))
      (args.at("room"))
      (args.at("eventname"))
      (args.at("speakerlogin")).exec();
    W.prepared("evaluation")
      (args.at("initial_evaluation"))
      (args.at("speakerlogin"))
      (args.at("talk")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::register_user_for_event(const ArgumentT &args) {
    if (!find_event(args.at("eventname")))
      throw std::runtime_error("Event doesn't exist");
    pqxx::work W(*C);
    W.prepared("register_user_for_event")
      (args.at("login"))
      (args.at("eventname")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::attendance(const ArgumentT &args) {
    if (get_talk_status(args.at("talk")) != TalkStatus::ACCEPTED)
      throw std::runtime_error("Talk doesn't exist");
    pqxx::work W(*C);
    W.prepared("attendance")
      (args.at("login"))
      (args.at("talk")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::evaluation(const ArgumentT &args) {
    if (get_talk_status(args.at("talk")) != TalkStatus::ACCEPTED)
      throw std::runtime_error("Talk doesn't exist");
    pqxx::work W(*C);
    W.prepared("evaluation")
      (args.at("rating"))
      (args.at("login"))
      (args.at("talk")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::reject(const ArgumentT &args) {
    if (get_talk_status(args.at("talk")) != TalkStatus::PROPOSED)
      throw std::runtime_error("Talk doesn't exist");
    pqxx::work W(*C);
    W.prepared("reject")
      (args.at("talk")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::proposal(const ArgumentT &args) {
    pqxx::work W(*C);
    W.prepared("proposal")
      (args.at("talk"))
      (args.at("title"))
      (args.at("start_timestamp"))
      (args.at("login")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::friends(const ArgumentT &args) {
    pqxx::work W(*C);
    W.prepared("friends")
      (args.at("login1"))
      (args.at("login2")).exec();
    W.commit();
    return {};
  }

  HandlerResultT database::user_plan(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("user_plan")
      (args.at("login")).exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }

  HandlerResultT database::day_plan(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("day_plan")
      (args.at("timestamp")).exec();
    return result_to_json(R, 0);
  }

  HandlerResultT database::best_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    pqxx::result R;
    if (std::stoi(args.at("all")))
      R = W.prepared("best_talks_all")
        (args.at("start_timestamp"))
        (args.at("end_timestamp")).exec();
    else R = W.prepared("best_talks")
        (args.at("start_timestamp"))
        (args.at("end_timestamp")).exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }

  HandlerResultT database::most_popular_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("most_popular_talks")
      (args.at("start_timestamp"))
      (args.at("end_timestamp")).exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }

  HandlerResultT database::attended_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("attended_talks")
      (args.at("login")).exec();
    return result_to_json(R, 0);
  }

  HandlerResultT database::abandoned_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("abandoned_talks").exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }

  HandlerResultT database::recently_added_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("recently_added_talks").exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }

  HandlerResultT database::rejected_talks(const ArgumentT &args) {
    UserStatus status = get_user_status(args.at("login"), args.at("password"));
    pqxx::work W(*C);
    pqxx::result R;
    if (status == UserStatus::ORGANIZER) {
      R = W.prepared("rejected_talks").exec();
    }
    else if (status == UserStatus::USER) {
      R = W.prepared("rejected_talks_usr")
        (args.at("login")).exec();
    }
    else throw std::runtime_error("User doesn't exist");
    return result_to_json(R, 0);
  }

  HandlerResultT database::proposals(__attribute__((unused)) const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("proposals").exec();
    return result_to_json(R, 0);
  }

  HandlerResultT database::friends_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("friends_talks")
      (args.at("login"))
      (args.at("start_timestamp"))
      (args.at("end_timestamp")).exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }

  HandlerResultT database::friends_events(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("friends_events")
      (args.at("login"))
      (args.at("eventname")).exec();
    return result_to_json(R, 0);
  }

  HandlerResultT database::recommended_talks(const ArgumentT &args) {
    pqxx::work W(*C);
    const pqxx::result R = W.prepared("recommended_talks")
      (args.at("login"))
      (args.at("start_timestamp"))
      (args.at("end_timestamp")).exec();
    return result_to_json(R, std::stoi(args.at("limit")));
  }
} //db
