extern crate diesel;
extern crate dotenv;

pub mod user;

use diesel::prelude::*;
use diesel::pg::PgConnection;
use dotenv::dotenv;
use std::env;
use self::user::{User, NewUser};

pub fn establish_connection() -> PgConnection {
    dotenv().ok();

    let database_url = env::var("DATABASE_URL")
        .expect("DATABASE_URL must be set");
    PgConnection::establish(&database_url)
        .expect(&format!("Error connecting to {}", database_url))
}

pub fn create_user<'a>(conn: &PgConnection, name: &'a str, password: &'a str, email: &'a str, verified: &'a bool) -> User {
    use self::user::users;

    let new_user = NewUser {
        name: name,
        password: password,
        email: email,
        verified: verified,
    };

    diesel::insert(&new_user).into(users::table)
        .get_result(conn)
        .expect("Error saving new user")
}
