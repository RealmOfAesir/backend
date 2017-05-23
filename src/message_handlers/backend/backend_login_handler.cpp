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

#include "backend_login_handler.h"
#include <messages/user_access_control/login_message.h>
#include <messages/user_access_control/login_response_message.h>
#include <easylogging++.h>
#include <sodium.h>

using namespace std;
using namespace roa;

static inline login_response_message<false> create_message(uint32_t client_id, uint32_t server_id, int8_t admin_status, int error_no, string error_msg) {
    return login_response_message<false>{
            {false, client_id, server_id, 0 /* ANY */},
            admin_status,
            error_no,
            error_msg
    };
}

backend_login_handler::backend_login_handler(Config config, iusers_repository &user_repository, std::shared_ptr<ikafka_producer<false>> producer)
        : _config(config), _user_repository(user_repository), _producer(producer) {

}

void backend_login_handler::handle_message(unique_ptr<message<false> const> const &msg) {
    string queue_name = "server-" + to_string(msg->sender.server_origin_id);
    try {
        if (auto login_msg = dynamic_cast<login_message<false> const *>(msg.get())) {
            char hashed_password[crypto_pwhash_STRBYTES];

            if(crypto_pwhash_str(hashed_password,
                                 login_msg->password.c_str(),
                                 login_msg->password.length(),
                                 crypto_pwhash_OPSLIMIT_MODERATE,
                                 crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
                LOG(ERROR) << "logging in user, but out of memory";
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "Something went wrong"));
            }

            auto transaction = _user_repository.create_transaction();
            STD_OPTIONAL<user> usr = _user_repository.get_user(login_msg->username, transaction);
            if(!usr) {
                LOG(DEBUG) << "Login " << login_msg->username << " doesn't exist";
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "User doesn't exist"));
            } else {
                LOG(DEBUG) << "Login " << login_msg->username;
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, usr->admin, 0, ""));
            }
        } else {
            LOG(ERROR) << "Couldn't cast message to login_message";
            this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "Something went wrong"));
        }
    } catch (std::runtime_error const &e) {
        LOG(ERROR) << "error: " << typeid(e).name() << "-" << e.what();
        this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "Something went wrong"));
    }
}

uint32_t constexpr backend_login_handler::message_id;