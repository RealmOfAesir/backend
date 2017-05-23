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

#include <pqxx/pqxx>
#include <easylogging++.h>

#include <sql_exceptions.h>

using namespace std;
using namespace experimental;
using namespace roa;
using namespace pqxx;

users_repository::users_repository(idatabase_pool& database_pool) : repository(database_pool) {

}

users_repository::users_repository(users_repository &repo) : repository(repo._database_pool, unique_ptr<idatabase_connection>(repo._connection.release())) {

}

users_repository::users_repository(users_repository &&repo) : repository(repo._database_pool, move(repo._connection)) {

}

users_repository::~users_repository() {

}

unique_ptr<idatabase_transaction> users_repository::create_transaction() {
    return repository::create_transaction();
}

bool users_repository::insert_user_if_not_exists(user& usr, std::unique_ptr<idatabase_transaction> const &transaction) {
    auto result = transaction->execute(
            "INSERT INTO users (username, password, email, login_attempts, admin) VALUES ('" + transaction->escape(usr.username) +
            "', '" + transaction->escape(usr.password) + "', '" + transaction->escape(usr.email) +
            "', " + to_string(usr.login_attempts) + ", " + to_string(usr.admin) + ") ON CONFLICT DO NOTHING RETURNING id");

    LOG(DEBUG) << "insert_user contains " << result.size() << " entries";

    if(result.size() == 0) {
        //already exists
        return false;
    }

    usr.id = result[0][0].as<uint64_t>();

    return true;
}

void users_repository::update_user(user const & usr, std::unique_ptr<idatabase_transaction> const &transaction) {
    auto result = transaction->execute("UPDATE users SET username = '" + transaction->escape(usr.username) +
             "', password = '" + transaction->escape(usr.password) + "', email = '" + transaction->escape(usr.email) +
             "', login_attempts = " + to_string(usr.login_attempts) + ", admin = " + to_string(usr.admin) + " WHERE id = " + to_string(usr.id));
}

STD_OPTIONAL<user> users_repository::get_user(string const & username, std::unique_ptr<idatabase_transaction> const &transaction) {
    auto result = transaction->execute("SELECT * FROM users WHERE username = '" + transaction->escape(username) + "'");

    LOG(DEBUG) << "get_user username contains " << result.size() << " entries";

    if(result.size() == 0) {
        return {};
    }

    return make_optional<user>({result[0]["id"].as<uint64_t>(), result[0]["username"].as<string>(),
                result[0]["password"].as<string>(), result[0]["email"].as<string>(),
                // sadly, int8_t is not implemented in pqxx. postgres field is smallint though.
                (int8_t)result[0]["login_attempts"].as<int32_t>(), (int8_t)result[0]["admin"].as<int32_t>()});
}

STD_OPTIONAL<user> users_repository::get_user(uint64_t id, std::unique_ptr<idatabase_transaction> const &transaction) {
    auto result = transaction->execute("SELECT * FROM users WHERE id = " + to_string(id));

    LOG(DEBUG) << "get_user id contains " << result.size() << " entries";

    if(result.size() == 0) {
        return {};
    }

    return make_optional<user>({result[0]["id"].as<uint64_t>(), result[0]["username"].as<string>(),
                result[0]["password"].as<string>(), result[0]["email"].as<string>(),
                // sadly, int8_t is not implemented in pqxx. postgres field is smallint though.
                (int8_t)result[0]["login_attempts"].as<int32_t>(), (int8_t)result[0]["admin"].as<int32_t>()});
}
