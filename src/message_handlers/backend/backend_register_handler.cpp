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

#include "backend_register_handler.h"
#include <easylogging++.h>
#include <messages/user_access_control/register_message.h>
#include <messages/user_access_control/register_response_message.h>
#include <sodium.h>

using namespace std;
using namespace roa;

static inline register_response_message<false> create_message(uint32_t client_id, uint32_t server_id, int8_t admin_status, int error_no, string error_msg) {
    return register_response_message<false>{
            {false, client_id, server_id, 0 /* ANY */},
            admin_status,
            error_no,
            error_msg
    };
}

backend_register_handler::backend_register_handler(Config config, iusers_repository &users_repository, ibanned_users_repository& banned_users_repository, std::shared_ptr<ikafka_producer<false>> producer)
        : _config(config), _users_repository(users_repository), _banned_users_repository(banned_users_repository), _producer(producer) {

}

void backend_register_handler::handle_message(unique_ptr<message<false> const> const &msg) {
    string queue_name = "server-" + to_string(msg->sender.server_origin_id);
    try {
        if (auto register_msg = dynamic_cast<register_message<false> const *>(msg.get())) {
            auto transaction = _users_repository.create_transaction();
            auto banned_user = _banned_users_repository.is_username_or_ip_banned(register_msg->username, register_msg->ip, transaction);

            if(banned_user) {
                LOG(INFO) << "logging in user, but is banned";
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -2, "You are banned"));
                return;
            }

            char hashed_password[crypto_pwhash_STRBYTES];

            if(crypto_pwhash_str(hashed_password,
                                 register_msg->password.c_str(),
                                 register_msg->password.length(),
                                 crypto_pwhash_OPSLIMIT_MODERATE,
                                 crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
                LOG(ERROR) << "Registering user, but out of memory";
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "Something went wrong"));
                return;
            }

            user usr{0, register_msg->username, string(hashed_password), register_msg->email, 0};
            if(!_users_repository.insert_user_if_not_exists(usr, transaction)) {
                LOG(DEBUG) << "Registering " << usr.username << " already exists";
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "User already exists"));
            } else {
                LOG(DEBUG) << "Registered user " << usr.username;
                this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, usr.admin, 0, ""));
            }
        } else {
            LOG(ERROR) << "Couldn't cast message to register_message";
            this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "Something went wrong"));
        }
    } catch (std::runtime_error const &e) {
        LOG(ERROR) << "error: " << typeid(e).name() << "-" << e.what();
        this->_producer->enqueue_message(queue_name, create_message(msg->sender.client_id, _config.server_id, 0, -1, "Something went wrong"));
    }
}

uint32_t constexpr backend_register_handler::message_id;