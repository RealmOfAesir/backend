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

#include <easylogging++.h>
#include <json.hpp>
#include <kafka_consumer.h>
#include <kafka_producer.h>
#include <admin_messages/admin_quit_message.h>

#include <signal.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include <thread>
#include <unordered_map>
#include <atomic>
#include <exceptions.h>
#include "message_handlers/backend/backend_quit_handler.h"
#include <roa_di.h>
#include <macros.h>
#include "message_handlers/backend/backend_register_handler.h"
#include "message_handlers/backend/backend_login_handler.h"
#include "repositories/users_repository.h"
#include "database_transaction.h"
#include "config.h"

using namespace std;
using namespace roa;
using namespace pqxx;

#ifdef EXPERIMENTAL_OPTIONAL
using namespace experimental;
#endif

using json = nlohmann::json;


INITIALIZE_EASYLOGGINGPP

atomic<bool> quit{false};

void on_sigint(int sig) {
    quit = true;
}

void init_extras() noexcept {
    ios::sync_with_stdio(false);
    signal(SIGINT, on_sigint);
}

void init_logger(Config const config) noexcept {
    el::Configurations defaultConf;
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level: %msg");
    if(!config.debug_level.empty()) {
        if(config.debug_level == "error") {
            defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "false");
            defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
            defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
            defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        } else if(config.debug_level == "warning") {
            defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
            defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
            defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        } else if(config.debug_level == "info") {
            defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
            defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        } else if(config.debug_level == "debug") {
            defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        }
    }
    el::Loggers::reconfigureAllLoggers(defaultConf);
    LOG(INFO) << "debug level: " << config.debug_level;
}



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
        config.broker_list = env_json["BROKER_LIST"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] BROKER_LIST missing in .env file.";
        exit(1);
    }

    try {
        config.group_id = env_json["GROUP_ID"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] GROUP_ID missing in .env file.";
        exit(1);
    }

    try {
        config.server_id = env_json["SERVER_ID"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] SERVER_ID missing in .env file.";
        exit(1);
    }

    if(config.server_id == 0) {
        LOG(ERROR) << "[main] SERVER_ID has to be greater than 0";
        exit(1);
    }

    try {
        config.connection_string = env_json["CONNECTION_STRING"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] CONNECTION_STRING missing in .env file.";
        exit(1);
    }

    try {
        config.debug_level = env_json["DEBUG_LEVEL"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] DEBUG_LEVEL missing in .env file.";
        exit(1);
    }

    return config;
}

int main() {
    Config config;
    try {
        config = parse_env_file();
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] .env file is malformed json.";
        exit(1);
    }

    init_logger(config);
    init_extras();

    database_pool db_pool;
    db_pool.create_connections(config.connection_string);
    auto common_injector = create_common_di_injector();
    auto backend_injector = boost::di::make_injector(
        boost::di::bind<idatabase_transaction>.to<database_transaction>(),
        boost::di::bind<idatabase_connection>.to<database_connection>(),
        boost::di::bind<idatabase_pool>.to(db_pool),
        boost::di::bind<iusers_repository>.to<users_repository>(),
        boost::di::bind<ibanned_users_repository>.to<banned_users_repository>());

    auto producer = common_injector.create<shared_ptr<ikafka_producer<false>>>();
    auto backend_consumer = common_injector.create<shared_ptr<ikafka_consumer<false>>>();
    auto users_repo = backend_injector.create<users_repository>();
    auto banned_users_repo = backend_injector.create<banned_users_repository>();
    backend_consumer->start(config.broker_list, config.group_id, std::vector<std::string>{"server-" + to_string(config.server_id), "backend_messages", "broadcast"});
    producer->start(config.broker_list);


    try {
        message_dispatcher<false> backend_server_msg_dispatcher;

        backend_server_msg_dispatcher.register_handler<backend_quit_handler>(&quit);
        backend_server_msg_dispatcher.register_handler<backend_register_handler, Config, iusers_repository&, ibanned_users_repository&, shared_ptr<ikafka_producer<false>>>(config, users_repo, banned_users_repo, producer);
        backend_server_msg_dispatcher.register_handler<backend_login_handler, Config, iusers_repository&, ibanned_users_repository&, shared_ptr<ikafka_producer<false>>>(config, users_repo, banned_users_repo, producer);

        LOG(INFO) << "[main] starting main thread";

        while (!quit) {
            try {
                producer->poll(10);
                auto msg = backend_consumer->try_get_message(10);
                if (get<1>(msg)) {

                    backend_server_msg_dispatcher.trigger_handler(msg);
                }
            } catch (serialization_exception &e) {
                cout << "[main] received exception " << e.what() << endl;
            }
        }

        LOG(INFO) << "[main] closing";

        producer->close();
        backend_consumer->close();
    } catch (const runtime_error& e) {
        LOG(ERROR) << "[main] error: " << typeid(e).name() << "-" << e.what();
    }

    return 0;
}
