/*
    Realm of Aesir backend
    Copyright (C) 2016  Michael de Lang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "users_repository.h"
#include "banned_users_repository.h"

#include <pqxx/pqxx>
#include <easylogging++.h>

#include <sql_exceptions.h>

using namespace std;
using namespace chrono;
using namespace experimental;
using namespace roa;
using namespace pqxx;

banned_users_repository::banned_users_repository(idatabase_pool &database_pool) : repository(database_pool) {

}

banned_users_repository::banned_users_repository(banned_users_repository &repo) : repository(repo._database_pool, unique_ptr<idatabase_connection>(repo._connection.release())) {

}

banned_users_repository::banned_users_repository(banned_users_repository &&repo) : repository(repo._database_pool, move(repo._connection)) {

}

banned_users_repository::~banned_users_repository() {

}

unique_ptr<idatabase_transaction> banned_users_repository::create_transaction() {
    return repository::create_transaction();
}

void banned_users_repository::insert_banned_user(banned_user &usr, std::unique_ptr<idatabase_transaction> const &transaction) {
    string ip = usr.ip.size() > 0 ? "'" + transaction->escape(usr.ip) + "'" : "NULL";
    string user_id = usr._user ? "'" + to_string(usr._user->id) + "'" : "NULL";
    string until = usr.until ? "to_timestamp(" + to_string(duration_cast<seconds>(usr.until->time_since_epoch()).count()) + ")" : "NULL";

    auto result = transaction->execute(
            "INSERT INTO banned_users (ip, user_id, until) VALUES (" + ip + ", " + user_id + ", " + until + ") RETURNING id");

    LOG(DEBUG) << "insert_banned_user contains " << result.size() << " entries";

    if(result.size() == 0) {
        LOG(ERROR) << "insert_banned_user unknown result";
        return;
    }

    usr.id = result[0][0].as<uint64_t>();
}

void banned_users_repository::update_banned_user(banned_user &usr, std::unique_ptr<idatabase_transaction> const &transaction) {

}

optional<banned_user> banned_users_repository::get_banned_user(string& username, std::unique_ptr<idatabase_transaction> const &transaction) {
    return optional<roa::banned_user>();

}

optional<banned_user> banned_users_repository::get_banned_user(uint64_t id, std::unique_ptr<idatabase_transaction> const &transaction) {
    auto result = transaction->execute("SELECT id, ip, user_id, extract(epoch from until)::bigint as until FROM banned_users "s +
                               "WHERE id = " + to_string(id));

    LOG(DEBUG) << "get_banned_user username contains " << result.size() << " entries";

    if(result.size() == 0) {
        return {};
    }

    string ip{};
    optional<user> _user;
    optional<steady_clock::time_point> until;

    if(!result[0]["ip"].is_null()) {
        ip = result[0]["ip"].as<string>();
    }

    if(!result[0]["user_id"].is_null()) {
        _user = make_optional<user>({result[0]["user_id"].as<uint64_t>()});
    }

    if(!result[0]["until"].is_null()) {
        until = steady_clock::time_point(seconds(result[0]["until"].as<int64_t>()));
    }

    return make_optional<banned_user>({result[0]["id"].as<uint64_t>(), ip, _user, until});
}
