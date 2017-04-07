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

#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include <thread>
#include <unordered_map>
#include <external/common/src/exceptions.h>

using namespace std;
using namespace roa;

using json = nlohmann::json;

INITIALIZE_EASYLOGGINGPP

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

    explicit Connection(uWS::WebSocket<uWS::SERVER> *ws) {
        this->state = UNKNOWN;
        this->ws = ws;
    }
};

void init_extras() noexcept {
    ios::sync_with_stdio(false);
}

void init_logger() noexcept {
    el::Configurations defaultConf;
    defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level: %msg");
    //defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
    el::Loggers::reconfigureAllLoggers(defaultConf);
}

json parse_env_file() {
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

    return json::parse(env_contents);
}

unique_ptr<thread> create_uws_thread(uWS::Hub &h, unique_ptr<kafka_producer<false>> &producer, unordered_map<string, Connection> &connections) {
    return make_unique<thread>([&h, &producer, &connections]{
        h.onMessage([&producer](uWS::WebSocket<uWS::SERVER> *ws, char *recv_msg, size_t length, uWS::OpCode opCode) {

            if(opCode == uWS::OpCode::TEXT) {
                LOG(WARNING) << "Got message from wss";
                string str(recv_msg, length);
                LOG(WARNING) << str;
                auto msg = message<true>::deserialize<false>(str);
                if (auto quit_msg = dynamic_cast<quit_message<false> *>(get<1>(msg).get())) {
                    LOG(WARNING) << "Got quit message from wss, sending quit message to kafka";
                    producer->enqueue_message(quit_msg);
                }
            } else {
                ws->send(recv_msg, length, opCode);
            }
        });

        h.onConnection([&connections](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest request) {
            LOG(WARNING) << "Got a connection";
            string key = AddressToString(ws->getAddress());
            if(connections.find(key) != connections.end()) {
                LOG(WARNING) << "Connection already present, closing this one";
                ws->terminate();
                return;
            }
            connections.insert(make_pair(key, Connection(ws)));
        });

        h.onDisconnection([&connections](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {

            string key = AddressToString(ws->getAddress());
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
    });
}

int main() {
    init_logger();
    init_extras();

    json env_json;
    try {
        env_json = parse_env_file();
    } catch (const std::exception& e) {
        LOG(ERROR) << "[main] .env file is malformed json.";
        exit(1);
    }

    string broker_list;
    try {
        broker_list = env_json["BROKER_LIST"];
    }   catch (const std::exception& e) {
        LOG(ERROR) << "[main] BROKER_LIST missing in .env file.";
        exit(1);
    }

    string group_id;
    try {
        group_id = env_json["GROUP_ID"];
    }   catch (const std::exception& e) {
        LOG(ERROR) << "[main] GROUP_ID missing in .env file.";
        exit(1);
    }

    string errstr;

    auto consumer = unique_ptr<kafka_consumer<false>>(new kafka_consumer<false>(broker_list, group_id, {"test_topic"}));
    auto producer = unique_ptr<kafka_producer<false>>(new kafka_producer<false>(broker_list, "test_topic"));
    unordered_map<string, Connection> connections;

    uWS::Hub h;
    bool quit = false;

    auto uws_thread = create_uws_thread(h, producer, connections);

    while(!quit) {
        try {
            producer->poll(10);
            auto msg = consumer->try_get_message(10);
            if(get<1>(msg)) {
                LOG(WARNING) << "Got message from kafka";

                if(get<0>(msg) == quit_message::id) {
                    LOG(WARNING) << "Got quit message from kafka";
                    quit = true;
                    continue;
                }
            }
        } catch (serialization_exception &e) {
            cout << "received exception " << e.what() << endl;
        }
    }

    return 0;
}
