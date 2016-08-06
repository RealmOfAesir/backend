extern crate diesel;
extern crate dotenv;

pub mod zone;
pub mod location;
pub mod user;

use diesel::prelude::*;
use diesel::pg::PgConnection;
use dotenv::dotenv;
use std::env;
use self::user::{User, NewUser, users};
use self::zone::{Zone, NewZone, zones};
use self::location::{Location, NewLocation, locations};

pub fn establish_connection() -> PgConnection {
    dotenv().ok();

    let database_url = env::var("DATABASE_URL")
        .expect("DATABASE_URL must be set");
    PgConnection::establish(&database_url)
        .expect(&format!("Error connecting to {}", database_url))
}

pub fn create_user<'a>(conn: &PgConnection, location_id: i32, name: &'a str, password: &'a str, email: &'a str, verification_code: &'a str) -> QueryResult<User> {
    let new_user = NewUser {
        location_id: location_id,
        name: name,
        password: password,
        email: email,
        verification_code: verification_code,
    };

    diesel::insert(&new_user).into(users::table)
        .get_result(conn)
}

pub fn create_zone<'a>(conn: &PgConnection, name: &'a str, data: &'a str) -> QueryResult<Zone> {
    let new_zone = NewZone {
        name: name,
        data: data,
    };

    diesel::insert(&new_zone).into(zones::table)
        .get_result(conn)
}

pub fn create_location<'a>(conn: &PgConnection, map_id: i32, x: i32, y: i32) -> QueryResult<Location> {
    let new_location = NewLocation {
        map_id: map_id,
        x: x,
        y: y,
    };

    diesel::insert(&new_location).into(locations::table)
        .get_result(conn)
}
