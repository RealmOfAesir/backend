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
#include <uWS.h>
#include <json.hpp>
#include <kafka_consumer.h>
#include <kafka_producer.h>
#include <admin_messages/quit_message.h>

#include <signal.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include <thread>
#include <unordered_map>
#include <atomic>
#include <shared_mutex>
#include <external/common/src/exceptions.h>
#include <src/message_handlers/server/server_quit_handler.h>
#include <external/common/src/roa_di.h>
#include <src/message_handlers/client/client_admin_quit_handler.h>
#include <src/sql_repositories/user_repository.h>
#include <src/sql_repositories/database_transaction.h>

using namespace std;
using namespace roa;
using namespace pqxx;

using json = nlohmann::json;

INITIALIZE_EASYLOGGINGPP

atomic<bool> quit{false};
atomic<bool> uwsQuit{false};
shared_timed_mutex connectionMutex;

string AddressToString(uS::Socket::Address &&a) {
    return string(a.address + to_string(a.port));
}

enum ConnectionState {
    UNKNOWN,
    LOGGED_IN
};

struct Connection {
    ConnectionState state;
    uWS::WebSocket<uWS::SERVER> *ws;
    int64_t id;
    static atomic<int64_t> idCounter;

    explicit Connection(uWS::WebSocket<uWS::SERVER> *ws) {
        this->state = UNKNOWN;
        this->ws = ws;
        this->id = idCounter.fetch_add(1, memory_order_relaxed);
    }
};

struct Config {
    string broker_list;
    string group_id;
    string connection_string;
};

atomic<int64_t> Connection::idCounter;

void on_sigint(int sig) {
    quit = true;
}

void init_extras() noexcept {
    ios::sync_with_stdio(false);
    signal(SIGINT, on_sigint);
}

void init_logger() noexcept {
    el::Configurations defaultConf;
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level: %msg");
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    el::Loggers::reconfigureAllLoggers(defaultConf);
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
        config.connection_string = env_json["CONNECTION_STRING"];
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] CONNECTION_STRING missing in .env file.";
        exit(1);
    }

    return config;
}

unique_ptr<thread> create_uws_thread(uWS::Hub &h, shared_ptr<kafka_producer<false>> producer, unordered_map<string, Connection> &connections) {
    return make_unique<thread>([&h, &producer, &connections]{

        message_dispatcher<false> client_msg_dispatcher;

        client_msg_dispatcher.register_handler<client_admin_quit_handler>(producer);

        h.onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *recv_msg, size_t length, uWS::OpCode opCode) {
            LOG(WARNING) << "Got message from wss";
            if(opCode == uWS::OpCode::TEXT) {
                LOG(WARNING) << "Got message from wss";
                string str(recv_msg, length);
                LOG(WARNING) << str;
                auto msg = message<true>::deserialize<false>(str);
                if(get<1>(msg)) {
                    client_msg_dispatcher.trigger_handler(msg);
                }
            } else {
                ws->send(recv_msg, length, opCode);
            }
        });

        h.onConnection([&connections](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest request) {
            LOG(WARNING) << "Got a connection";
            string key = AddressToString(ws->getAddress());
            unique_lock<shared_timed_mutex> lock(connectionMutex);
            if(connections.find(key) != connections.end()) {
                LOG(WARNING) << "Connection already present, closing this one";
                ws->terminate();
                return;
            }
            connections.insert(make_pair(key, Connection(ws)));
        });

        h.onDisconnection([&connections](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {

            string key = AddressToString(ws->getAddress());
            unique_lock<shared_timed_mutex> lock(connectionMutex);
            connections.erase(key);

            LOG(WARNING) << "Got a disconnect, " << connections.size() << " connections remaining";
        });

        h.onError([](int type) {
            LOG(WARNING) << "Got error:" << type;
        });

        //auto context = uS::TLS::createContext("cert.pem", "key.pem", "test");
        //h.getDefaultGroup<uWS::SERVER>().addAsync();
        if(!h.listen(3000/*, nullptr, 0, group.get()*/)) {
            LOG(ERROR) << "h.listen failed";
            return;
        }
        h.run();

        uwsQuit = true;
    });
}

int main() {
    init_logger();
    init_extras();

    Config config;
    try {
        config = parse_env_file();
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] .env file is malformed json.";
        exit(1);
    }

    database_pool db_pool;
    db_pool.create_connections(config.connection_string);
    auto common_injector = create_common_di_injector();
    auto backend_injector = boost::di::make_injector(
        boost::di::bind<idatabase_transaction>.to<database_transaction>(),
        boost::di::bind<idatabase_connection>.to<idatabase_connection>(),
        boost::di::bind<idatabase_pool>.to(db_pool),
        boost::di::bind<iuser_repository>.to<user_repository>());
    //auto injector = boost::di::make_injector(common_injector, backend_injector);

    user_repository repo = backend_injector.create<user_repository>();
    auto next_print = (chrono::system_clock::now() += 2000ms);
    uint32_t insert_count = 0;
    uint32_t id = 0;
    while(!quit) {
        repo.insert_user({id, "john doe"s, "pass"s, "email"s});

        insert_count++;
        id++;

        if(chrono::system_clock::now() >= next_print) {
            LOG(INFO) << insert_count << " inserts";
            insert_count = 0;
            next_print = (chrono::system_clock::now() += 2000ms);
        }
    }

    return 0;


    auto producer = common_injector.create<shared_ptr<kafka_producer<false>>>();
    auto consumer = common_injector.create<unique_ptr<kafka_consumer<false>>>();
    consumer->start(config.broker_list, config.group_id, std::vector<std::string>{"test_topic"});
    producer->start(config.broker_list, "test_topic");
    unordered_map<string, Connection> connections;

    uWS::Hub h;

    auto uws_thread = create_uws_thread(h, producer, connections);
    message_dispatcher<false> server_msg_dispatcher;

    server_msg_dispatcher.register_handler<server_quit_handler, atomic<bool>*>(&quit);

    while(!quit) {
        try {
            producer->poll(10);
            auto msg = consumer->try_get_message(10);
            if(get<1>(msg)) {
                LOG(WARNING) << "Got message from kafka";

                server_msg_dispatcher.trigger_handler(msg);
            }
        } catch (serialization_exception &e) {
            cout << "received exception " << e.what() << endl;
        }
    }

    LOG(INFO) << "closing";

    auto loop = h.getLoop();
    auto closeLambda = [](Async* as) -> void {
        uWS::Hub *hub = static_cast<uWS::Hub*>(as->data);
        hub->getLoop()->destroy();
    };
    Async async{loop};
    async.setData(&h);
    async.start(closeLambda);
    async.send();

    producer->close();
    consumer->close();

    auto now = chrono::system_clock::now().time_since_epoch().count();
    auto wait_until = (chrono::system_clock::now() += 2000ms).time_since_epoch().count();

    while(!uwsQuit && now < wait_until) {
        this_thread::sleep_for(100ms);
        now = chrono::system_clock::now().time_since_epoch().count();
    }

    async.close();

    return 0;
}
