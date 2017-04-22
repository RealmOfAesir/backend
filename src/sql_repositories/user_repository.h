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

#include <string>
#include "database_pool.h"

namespace roa {
    struct user {
        uint64_t id;
        std::string username;
        std::string password;
        std::string email;
        uint32_t login_attempts;
    };

    class iuser_repository {
    public:
        virtual ~iuser_repository() = default;

        virtual void insert_user(user usr) = 0;
        virtual void update_user(user usr) = 0;
        virtual user get_user(std::string username) = 0;
        virtual user get_user(uint64_t id) = 0;
    };

    class user_repository : public iuser_repository {
    public:
        user_repository(idatabase_pool& database_pool);
        ~user_repository();

        void insert_user(user usr);
        void update_user(user usr);
        user get_user(std::string username);
        user get_user(uint64_t id);

    private:
        idatabase_pool& _database_pool;
    };
}