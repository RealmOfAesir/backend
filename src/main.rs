#![feature(custom_derive, custom_attribute, plugin)]
#![plugin(diesel_codegen, dotenv_macros)]

#[macro_use]
extern crate diesel;
extern crate dotenv;

mod database;
use database::*;
use database::user::*;
use self::diesel::prelude::*;

fn main() {
    use database::user::users::dsl::*;

    let connection = establish_connection();

    let mut user = create_user(&connection, 1, "oipo", "haha_no", "youbetcha", "true");

    match user {
        Result::Ok(val) => println!("This should not happen!"),
        Result::Err(err) => println!("Correct! {:?}", err),
    }

    let zone = create_zone(&connection, "Test", "{}").expect("Could not create zone");
    let location = create_location(&connection, zone.id, 0, 0).expect("Could not create location");

    user = create_user(&connection, location.id, "oipo", "haha_no", "youbetcha", "true");

    match user {
        Result::Ok(val) => println!("Correct!"),
        Result::Err(err) => println!("This should not happen! {:?}", err),
    }

    let results = users.limit(5)
        .load::<User>(&connection)
        .expect("Error loading users");

    println!("Displaying {} users", results.len());
    for post in results {
        println!("{}", post.name);
        println!("----------\n");
        println!("{}", post.email);
    }
}
