/*
    Realm of Aesir
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

#include <catch.hpp>
#include <roa_di.h>
#include <easylogging++.h>
#include <database_transaction.h>
#include <json.hpp>
#include "../src/repositories/users_repository.h"
#include "../src/repositories/banned_users_repository.h"
#include "../src/config.h"

using namespace std;
using namespace roa;
using json = nlohmann::json;

Config parse_env_file() {
    string env_contents;
    ifstream env(".env");

    if(!env) {
        LOG(ERROR) << "[main] no .env file found. Please make one.";
        exit(1);
    }

    env.seekg(0, ios::end);
    env_contents.resize(env.tellg());
    env.seekg(0, ios::beg);
    env.read(&env_contents[0], env_contents.size());
    env.close();

    auto env_json = json::parse(env_contents);
    Config config;

    try {
        config.connection_string = env_json["CONNECTION_STRING"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] CONNECTION_STRING missing in .env file.";
        exit(1);
    }

    return config;
}

TEST_CASE("insert and retrieve banned user") {
    Config config;

    REQUIRE_NOTHROW(config = parse_env_file());
    REQUIRE(config.connection_string.size() > 0);

    database_pool db_pool;
    REQUIRE_NOTHROW(db_pool.create_connections(config.connection_string));

    auto backend_injector = boost::di::make_injector(
            boost::di::bind<idatabase_transaction>.to<database_transaction>(),
            boost::di::bind<idatabase_connection>.to<database_connection>(),
            boost::di::bind<idatabase_pool>.to(db_pool),
            boost::di::bind<irepository>.to<repository>(),
            boost::di::bind<ibanned_users_repository>.to<banned_users_repository>());

    banned_users_repository user_repo = backend_injector.create<banned_users_repository>();
    {
        auto transaction = user_repo.create_transaction();
        banned_user usr{0, "192.168.0.1"s, {}, chrono::steady_clock::now()};
        user_repo.insert_banned_user(usr, transaction);
        REQUIRE(usr.id != 0);

        auto usr2 = user_repo.get_banned_user(usr.id, transaction);
        REQUIRE(usr2);
        REQUIRE(usr2->id == usr.id);
        REQUIRE(usr2->ip == usr.ip);
        REQUIRE(!usr2->_user);
        REQUIRE(usr2->until);

        auto time1 = chrono::duration_cast<chrono::seconds>(usr.until->time_since_epoch()).count();
        auto time2 = chrono::duration_cast<chrono::seconds>(usr2->until->time_since_epoch()).count();

        REQUIRE(time1 == time2);
    }
}