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

#include "message_handler.h"
#include <tuple>

using namespace std;
using namespace roa;

template <bool UseJson>
void message_dispatcher<UseJson>::register_handler(uint32_t message_type, imessage_handler<UseJson> handler) {
    handlers[message_type].push_back(handler);
}

template <bool UseJson>
void message_dispatcher<UseJson>::trigger_handle(decltype(message<UseJson>::template deserialize<UseJson>) msg) {
    auto message_handlers = handlers[get<0>(msg)];
    for(auto msg_handler : message_handlers) {
        //msg_handler->
    }
}
