// Autor: Łukasz Siudek
// Indeks: 283493

#ifndef __DB_FUNCTIONS__
#define __DB_FUNCTIONS__

#include <string>
#include <functional>
#include <boost/optional.hpp>
#include "libs/json.hpp"

namespace db {
  using json = nlohmann::json;
  using HandlerResultT = boost::optional<json>;
  using ArgumentT = std::map<std::string, std::string>;
  using HandlerT = std::function<HandlerResultT (const ArgumentT&)>;

  const std::string find_user_sql =
    "SELECT * FROM usr WHERE "
    "login=$1 AND password=$2";
  const std::string find_talk_sql =
    "SELECT * FROM talk WHERE talk=$1";
  const std::string find_event_sql =
    "SELECT * FROM event WHERE name=$1";
  const std::string open_sql =
    "";
  const std::string organizer_sql =
    "INSERT INTO usr (login, password, status) "
    "values ($1, $2, 'organizer') ";
  const std::string event_sql =
    "INSERT INTO event (start_timestamp, end_timestamp, name) "
    "values ($1, $2, $3)";
  const std::string user_sql =
    "INSERT INTO usr (login, password) "
    "values ($1, $2)";
  const std::string talk_sql =
    "INSERT INTO talk (talk, title, start_timestamp, room, status, eventname, "
      "speakerlogin, accept_timestamp) "
    "values ($1, $2, $3, $4, 'accepted', $5, $6, now()) "
    "ON CONFLICT (talk) DO UPDATE "
    "SET room=$4, status='accepted', eventname=(CASE WHEN $5='' THEN NULL "
      "ELSE $5 END), accept_timestamp=now()";
  const std::string register_user_for_event_sql =
    "INSERT INTO registered (login, eventname) "
    "values ($1, $2)";
  const std::string attendance_sql =
    "INSERT INTO attendance (login, talk) "
    "values ($1, $2)";
  const std::string evaluation_sql =
    "INSERT INTO evaluation (rating, login, talk) "
    "values ($1, $2, $3) "
    "ON CONFLICT (login, talk) DO UPDATE "
    "SET rating=$1";
  const std::string reject_sql =
    "UPDATE proposed_talks SET status='rejected' WHERE talk=$1";
  const std::string proposal_sql =
    "INSERT INTO talk (talk, title, start_timestamp, speakerlogin) "
    "values ($1, $2, $3, $4)";
  const std::string friends_sql =
    "INSERT INTO friends (login, friendlogin) "
    "values ($1, $2)";
  const std::string user_plan_sql =
    "SELECT t.speakerlogin, t.talk, t.start_timestamp, t.title, t.room FROM usr p "
    "JOIN registered r ON (p.login = r.login) "
    "JOIN accepted_talks t ON (r.eventname = t.eventname) "
    "WHERE t.start_timestamp > now() AND p.login=$1 "
    "ORDER BY t.start_timestamp";
  const std::string day_plan_sql =
    "SELECT talk, start_timestamp, title, room FROM accepted_talks "
    "WHERE date(start_timestamp)=date($1) "
    "ORDER BY room, start_timestamp";
  const std::string best_talks_all_sql =
    "SELECT t.talk, t.start_timestamp, t.title, t.room FROM talk t "
    "JOIN evaluation e ON (t.talk = e.talk) "
    "WHERE t.start_timestamp BETWEEN $1 AND $2 "
    "GROUP BY t.talk, t.start_timestamp, t.title, t.room "
    "ORDER BY avg(rating) DESC";
  const std::string best_talks_sql =
    "SELECT t.talk, t.start_timestamp, t.title, t.room FROM "
      "((SELECT e.login, e.talk, e.rating FROM attendance a "
      "JOIN evaluation e ON (a.login=e.login AND a.talk=e.talk)) "
      "UNION "
      "(SELECT e.login, e.talk, e.rating FROM evaluation e "
      "JOIN usr u ON (e.login=u.login) WHERE status='organizer')) e2 "
    "JOIN talk t ON (e2.talk=t.talk) "
    "WHERE t.start_timestamp BETWEEN $1 AND $2 "
    "GROUP BY t.talk, t.start_timestamp, t.title, t.room "
    "ORDER BY avg(rating) DESC";
  const std::string most_popular_talks_sql =
    "SELECT t.talk, t.start_timestamp, t.title, t.room FROM accepted_talks t "
    "JOIN attendance a ON (t.talk = a.talk) "
    "WHERE start_timestamp BETWEEN $1 AND $2 "
    "GROUP BY t.talk, t.start_timestamp, t.title, t.room "
    "ORDER BY COUNT(a.login) DESC";
  const std::string attended_talks_sql =
    "SELECT t.talk, t.start_timestamp, t.title, t.room FROM accepted_talks t "
    "JOIN attendance a ON (t.talk=a.talk) WHERE a.login=$1";
  const std::string abandoned_talks_sql =
    "SELECT t.talk, t.start_timestamp, t.title, t.room FROM "
      "((SELECT r.login, t.talk, t.start_timestamp, t.title, t.room FROM "
      "registered r JOIN accepted_talks t ON (r.eventname = t.eventname)) "
      "EXCEPT "
      "(SELECT a.login, t.talk, t.start_timestamp, t.title, t.room FROM "
      "attendance a JOIN accepted_talks t ON (a.talk = t.talk))) t "
    "GROUP BY t.talk, t.start_timestamp, t.title, t.room "
    "ORDER BY count(t.login) DESC";
  const std::string recently_added_talks_sql =
    "SELECT talk, speakerlogin, start_timestamp, title, room "
    "FROM accepted_talks ORDER BY accept_timestamp DESC";
  const std::string rejected_talks_org_sql =
    "SELECT talk, speakerlogin, start_timestamp, title "
    "FROM rejected_talks";
  const std::string rejected_talks_usr_sql =
    "SELECT talk, speakerlogin, start_timestamp, title "
    "FROM rejected_talks WHERE speakerlogin=$1";
  const std::string proposals_sql =
    "SELECT talk, speakerlogin, start_timestamp, title "
    "FROM proposed_talks";
  const std::string friends_talks_sql =
    "SELECT talk, speakerlogin, start_timestamp, title, room "
    "FROM (SELECT * FROM made_friends f WHERE $1=f.login) mf "
    "JOIN talk t ON (t.speakerlogin=mf.friendlogin) "
    "WHERE t.start_timestamp BETWEEN $2 AND $3 "
    "ORDER BY t.start_timestamp";
  const std::string friends_events_sql =
    "SELECT f.login, r.eventname, f.friendlogin FROM registered r "
    "JOIN made_friends f ON (f.friendlogin=r.login) "
    "WHERE f.login=$1 AND r.eventname=$2";
  const std::string recommended_talks_sql =
    "SELECT t.talk, t.speakerlogin, t.start_timestamp, t.title, t.room"
    "FROM talk t WHERE t.start_timestamp BETWEEN $2 AND $3 "
    "ORDER BY score($1, t.talk) DESC";

  enum class UserStatus {
    NONE,
    USER,
    ORGANIZER
  };

  enum class TalkStatus {
    NONE,
    PROPOSED,
    ACCEPTED,
    REJECTED
  };

  struct Function {
    HandlerT handler;
    UserStatus auth_level;
    std::string sql;
  };

  HandlerResultT result_to_json(const pqxx::result &R, int limit);


class database {
private:
  HandlerT wrap_handler(HandlerResultT(database::*handler)(const ArgumentT&)) {
    return std::bind(handler, this, std::placeholders::_1);
  }

  void prepare();
  UserStatus get_user_status(const std::string &login, const std::string &password);
  TalkStatus get_talk_status(const std::string &talk);
  bool find_event(const std::string &eventname);
  void verify_authorization(const Function &function, const ArgumentT &args);
  HandlerResultT open(const ArgumentT &args);
  HandlerResultT organizer(const ArgumentT &args);
  HandlerResultT event(const ArgumentT &args);
  HandlerResultT user(const ArgumentT &args);
  HandlerResultT talk(const ArgumentT &args);
  HandlerResultT register_user_for_event(const ArgumentT &args);
  HandlerResultT attendance(const ArgumentT &args);
  HandlerResultT evaluation(const ArgumentT &args);
  HandlerResultT reject(const ArgumentT &args);
  HandlerResultT proposal(const ArgumentT &args);
  HandlerResultT friends(const ArgumentT &args);
  HandlerResultT user_plan(const ArgumentT &args);
  HandlerResultT day_plan(const ArgumentT &args);
  HandlerResultT best_talks(const ArgumentT &args);
  HandlerResultT most_popular_talks(const ArgumentT &args);
  HandlerResultT attended_talks(const ArgumentT &args);
  HandlerResultT abandoned_talks(const ArgumentT &args);
  HandlerResultT recently_added_talks(const ArgumentT &args);
  HandlerResultT rejected_talks(const ArgumentT &args);
  HandlerResultT proposals(const ArgumentT &args);
  HandlerResultT friends_talks(const ArgumentT &args);
  HandlerResultT friends_events(const ArgumentT &args);
  HandlerResultT recommended_talks(const ArgumentT &args);

  const std::map<std::string, Function> functions = {
    {"find_user", {nullptr, UserStatus::NONE, find_user_sql}},
    {"find_talk", {nullptr, UserStatus::NONE, find_talk_sql}},
    {"find_event", {nullptr, UserStatus::NONE, find_event_sql}},
    {"open", {wrap_handler(&database::open), UserStatus::NONE, open_sql}},
    {"organizer", {wrap_handler(&database::organizer), UserStatus::NONE, organizer_sql}},
    {"event", {wrap_handler(&database::event), UserStatus::ORGANIZER, event_sql}},
    {"user", {wrap_handler(&database::user), UserStatus::ORGANIZER, user_sql}},
    {"talk", {wrap_handler(&database::talk), UserStatus::ORGANIZER, talk_sql}},
    {"register_user_for_event", {wrap_handler(&database::register_user_for_event), UserStatus::USER, register_user_for_event_sql}},
    {"attendance", {wrap_handler(&database::attendance), UserStatus::USER, attendance_sql}},
    {"evaluation", {wrap_handler(&database::evaluation), UserStatus::USER, evaluation_sql}},
    {"reject", {wrap_handler(&database::reject), UserStatus::ORGANIZER, reject_sql}},
    {"proposal", {wrap_handler(&database::proposal), UserStatus::USER, proposal_sql}},
    {"friends", {wrap_handler(&database::friends), UserStatus::USER, friends_sql}},
    {"user_plan", {wrap_handler(&database::user_plan), UserStatus::NONE, user_plan_sql}},
    {"day_plan", {wrap_handler(&database::day_plan), UserStatus::NONE, day_plan_sql}},
    {"best_talks", {wrap_handler(&database::best_talks), UserStatus::NONE, best_talks_sql}},
    {"best_talks_all", {nullptr, UserStatus::NONE, best_talks_all_sql}},
    {"most_popular_talks", {wrap_handler(&database::most_popular_talks), UserStatus::NONE, most_popular_talks_sql}},
    {"attended_talks", {wrap_handler(&database::attended_talks), UserStatus::USER, attended_talks_sql}},
    {"abandoned_talks", {wrap_handler(&database::abandoned_talks), UserStatus::ORGANIZER, abandoned_talks_sql}},
    {"recently_added_talks", {wrap_handler(&database::recently_added_talks), UserStatus::NONE, recently_added_talks_sql}},
    {"rejected_talks", {wrap_handler(&database::rejected_talks), UserStatus::NONE, rejected_talks_org_sql}},
    {"rejected_talks_usr", {nullptr, UserStatus::NONE, rejected_talks_usr_sql}},
    {"proposals", {wrap_handler(&database::proposals), UserStatus::ORGANIZER, proposals_sql}},
    {"friends_talks", {wrap_handler(&database::friends_talks), UserStatus::USER, friends_talks_sql}},
    {"friends_events", {wrap_handler(&database::friends_events), UserStatus::USER, friends_events_sql}},
    {"recommended_talks", {wrap_handler(&database::recommended_talks), UserStatus::USER, recommended_talks_sql}}
  };

  public:
    HandlerResultT run(const std::string &command_name, const ArgumentT &args);
};

} // db

#endif
