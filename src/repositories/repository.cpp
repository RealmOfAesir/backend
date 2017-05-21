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

#include "users_repository.h"
#include "banned_users_repository.h"

#include <pqxx/pqxx>
#include <easylogging++.h>

#include <sql_exceptions.h>

using namespace std;
using namespace experimental;
using namespace roa;
using namespace pqxx;

repository::repository(idatabase_pool& database_pool, std::unique_ptr<idatabase_connection> connection)
        : _database_pool(database_pool), _connection(move(connection)) {

}

repository::~repository() {

}

unique_ptr<idatabase_transaction> repository::create_transaction()  {
    if(!_connection) {
        _connection = _database_pool.get_connection();
    }

    return _connection->create_transaction();
}
