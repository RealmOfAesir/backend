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

    let actuallyTrue = true;
    let user = create_user(&connection, "oipo", "haha_no", "youbetcha", &actuallyTrue);

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
