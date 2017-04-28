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

#include "user_repository.h"

#include <pqxx/pqxx>
#include <easylogging++.h>

#include "database_connection.h"
#include "database_transaction.h"
#include "sql_exceptions.h"

using namespace std;
using namespace experimental;
using namespace roa;
using namespace pqxx;

user_repository::user_repository(idatabase_pool& database_pool) : _database_pool(database_pool) {

}

user_repository::~user_repository() {

}

bool user_repository::insert_user_if_not_exists(user& usr) {
    auto connection = _database_pool.get_connection();
    auto txn = connection->create_transaction();

    auto result = txn->execute(
            "INSERT INTO users (username, password, email, login_attempts) VALUES ('" + txn->escape(usr.username) +
            "', '" + txn->escape(usr.password) + "', '" + txn->escape(usr.email) + "', 0) ON CONFLICT DO NOTHING RETURNING id");

    if(result.size() == 0) {
        //already exists
        return false;
    }

    usr.id = result[0][0].as<uint64_t>();

    LOG(DEBUG) << "insert_user contains " << result.size() << " entries";

    txn->commit();

    return true;
}

void user_repository::update_user(user usr) {
    auto connection = _database_pool.get_connection();
    auto txn = connection->create_transaction();

    auto result = txn->execute("UPDATE users SET username = '" + txn->escape(usr.username) +
             "', password = '" + txn->escape(usr.password) + "', email = '" + txn->escape(usr.email) + "' WHERE id = " + to_string(usr.id));

    LOG(DEBUG) << "update_user contains " << result.size() << " entries";

    txn->commit();
}

STD_OPTIONAL<user> user_repository::get_user(string username) {
    auto connection = _database_pool.get_connection();
    auto txn = connection->create_transaction();

    auto result = txn->execute("SELECT * FROM users WHERE username = '" + txn->escape(username) + "'");

    LOG(DEBUG) << "get_user username contains " << result.size() << " entries";

    if(result.size() == 0) {
        return {};
    }

    return make_optional<user>({result[0]["id"].as<uint64_t>(), result[0]["username"].as<string>(),
                result[0]["password"].as<string>(), result[0]["email"].as<string>(),
                result[0]["login_attempts"].as<int32_t>(), result[0]["admin_status"].as<int32_t>()});
}

STD_OPTIONAL<user> user_repository::get_user(uint64_t id) {
    auto connection = _database_pool.get_connection();
    auto txn = connection->create_transaction();

    auto result = txn->execute("SELECT * FROM users WHERE id = " + to_string(id));

    LOG(DEBUG) << "get_user id contains " << result.size() << " entries";

    if(result.size() == 0) {
        return {};
    }

    return make_optional<user>({result[0]["id"].as<uint64_t>(), result[0]["username"].as<string>(),
                result[0]["password"].as<string>(), result[0]["email"].as<string>(),
                result[0]["login_attempts"].as<int32_t>(), result[0]["admin_status"].as<int32_t>()});
}
