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
#include <custom_optional.h>
#include "database_pool.h"

namespace roa {
    struct user {
        uint64_t id;
        std::string username;
        std::string password;
        std::string email;
        int8_t login_attempts;
        int8_t admin_status;
    };

    class iuser_repository {
    public:
        virtual ~iuser_repository() = default;

        /**
         * Insert a user
         * @param usr Reference to user, id gets set on succesful insertion
         * @return true if insert, false if not
         */
        virtual bool insert_user_if_not_exists(user& usr) = 0;

        /**
         * Update a user
         * @param usr
         */
        virtual void update_user(user usr) = 0;

        /**
         * Get user by username
         * @param username
         * @return user
         * @throws not_found_exception
         */
        virtual STD_OPTIONAL<user> get_user(std::string username) = 0;

        /**
         * Get user by id
         * @param id
         * @return user
         * @throws not_found_exception
         */
        virtual STD_OPTIONAL<user> get_user(uint64_t id) = 0;
    };

    class user_repository : public iuser_repository {
    public:
        user_repository(idatabase_pool& database_pool);
        ~user_repository();

        bool insert_user_if_not_exists(user& usr) override;
        void update_user(user usr) override;
        STD_OPTIONAL<user> get_user(std::string username) override;
        STD_OPTIONAL<user> get_user(uint64_t id) override;

    private:
        idatabase_pool& _database_pool;
    };
}