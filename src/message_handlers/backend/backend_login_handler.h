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
#include "../../sql_repositories/user_repository.h"
#include <kafka_producer.h>
#include "../../config.h"

namespace roa {
    class backend_login_handler : public imessage_handler<false> {
    public:
        backend_login_handler(Config config, iuser_repository& user_repository, std::shared_ptr<ikafka_producer<false>> producer);
        ~backend_login_handler() override = default;

        void handle_message(std::unique_ptr<message<false> const> const &msg, STD_OPTIONAL<std::reference_wrapper<user_connection>> connection) override;

        static constexpr uint32_t message_id = LOGIN_MESSAGE_TYPE;
    private:
        Config _config;
        iuser_repository& _user_repository;
        std::shared_ptr<ikafka_producer<false>> _producer;
    };
}
