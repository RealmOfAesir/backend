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

#include <external/common/src/messages/message_handler.h>

#include <atomic>

namespace roa {
    class server_quit_handler : public imessage_handler<false> {
    public:
        //server_quit_handler();
        void handle_message(std::unique_ptr<message<false> const> const &msg) override;

        server_quit_handler(std::atomic<bool> *quit);
        ~server_quit_handler() override = default;

        static constexpr uint32_t message_id = ADMIN_QUIT_MESSAGE_TYPE;
    private:
        std::atomic<bool> *_quit;
    };
}
