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

#include "client_admin_quit_handler.h"
#include <external/common/src/admin_messages/quit_message.h>
#include <easylogging++.h>

using namespace std;
using namespace roa;

client_admin_quit_handler::client_admin_quit_handler(std::shared_ptr<ikafka_producer<false>> producer) : _producer(producer) {

}

void client_admin_quit_handler::handle_message(const unique_ptr<const message<false>> &msg) {
    // TODO check if user is admin
    if (auto quit_msg = dynamic_cast<quit_message<false> const *>(msg.get())) {
        LOG(WARNING) << "Got quit message from wss, sending quit message to kafka";
        this->_producer->enqueue_message(quit_msg);
    }
}

uint32_t constexpr client_admin_quit_handler::message_id;
