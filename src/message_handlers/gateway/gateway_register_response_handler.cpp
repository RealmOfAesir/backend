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

#include "gateway_register_response_handler.h"
#include <macros.h>
#include <easylogging++.h>
#include <messages/user_access_control/register_response_message.h>

using namespace std;
using namespace roa;

gateway_register_response_handler::gateway_register_response_handler(Config config,
                                                                     std::unordered_map<std::string, user_connection> &connections)
    : _config(config), _connections(connections) {

}

void gateway_register_response_handler::handle_message(std::unique_ptr<message<false> const> const &msg,
                                                       STD_OPTIONAL<std::reference_wrapper<user_connection>> connection) {
    if(!connection) {
        LOG(ERROR) << NAMEOF(gateway_register_response_handler::handle_message) << " received empty connection";
        return;
    }

    if (auto response_msg = dynamic_cast<register_response_message<false> const *>(msg.get())) {
        LOG(DEBUG) << NAMEOF(gateway_register_response_handler::handle_message) << "Got response message from backend";

        if(response_msg->error_number != 0) {
            register_response_message<true> response{{false, 0, 0, 0}, 0, response_msg->error_number, response_msg->error_str};
            auto response_str = response.serialize();
            connection->get().ws->send(response_str.c_str(), response_str.length(), uWS::OpCode::TEXT);
        } else {
            connection->get().state = user_connection_state::LOGGED_IN;
            connection->get().admin_status = response_msg->admin_status;
            register_response_message<true> response{{false, 0, 0, 0}, 0, 0, ""};
            auto response_str = response.serialize();
            connection->get().ws->send(response_str.c_str(), response_str.length(), uWS::OpCode::TEXT);
        }

    } else {
        LOG(ERROR) << NAMEOF(gateway_register_response_handler::handle_message) << "Couldn't cast message to register_response_message";
        register_response_message<true> response{{false, 0, 0, 0}, 0, -1, "Something went wrong."};
        auto response_str = response.serialize();
        connection->get().ws->send(response_str.c_str(), response_str.length(), uWS::OpCode::TEXT);
    }
}

uint32_t constexpr gateway_register_response_handler::message_id;