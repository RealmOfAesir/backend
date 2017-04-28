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

#pragma once

#include "../message_dispatcher.h"
#include <kafka_producer.h>
#include "src/user_connection.h"
#include "../../config.h"

namespace roa {
    class client_register_handler : public imessage_handler<false> {
    public:
        client_register_handler(Config config,
                                std::shared_ptr<ikafka_producer<false>> producer,
                                std::unordered_map<std::string, user_connection> &connections);
        ~client_register_handler() override = default;

        void handle_message(std::unique_ptr<message<false> const> const &msg, STD_OPTIONAL<std::reference_wrapper<user_connection>> connection) override;

        static constexpr uint32_t message_id = REGISTER_MESSAGE_TYPE;
    private:
        Config _config;
        std::shared_ptr<ikafka_producer<false>> _producer;
        std::unordered_map<std::string, user_connection> &_connections;
    };
}