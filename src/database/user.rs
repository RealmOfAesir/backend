table! {
    users {
        id -> Integer,
        location_id -> Integer,
        name -> Text,
        password -> Text,
        email -> Text,
        verification_code -> Text,
    }
}

#[derive(Queryable)]
pub struct User {
    pub id: i32,
    pub location_id: i32,
    pub name: String,
    pub password: String,
    pub email: String,
    pub verification_code: String,
}

#[insertable_into(users)]
pub struct NewUser<'a> {
    pub location_id: i32,
    pub name: &'a str,
    pub password: &'a str,
    pub email: &'a str,
    pub verification_code: &'a str,
}
