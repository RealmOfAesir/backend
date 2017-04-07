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

#include <unordered_map>
#include <vector>
#include <external/common/src/messages/message.h>

namespace roa {

    template <bool UseJson>
    class imessage_handler {
    public:
        virtual ~imessage_handler() = default;
        virtual void handle_message(message<UseJson> msg) = 0;
    };

    template <bool UseJson>
    class message_dispatcher {
    public:
        message_dispatcher() : handlers() {}

        void register_handler(uint32_t message_type, imessage_handler<UseJson> handler);
        void trigger_handle(decltype(message<UseJson>::template deserialize<UseJson>) message);
    private:
        std::unordered_map<uint32_t, std::vector<imessage_handler<UseJson>>> handlers;
    };
}