table! {
    users {
        id -> Integer,
        name -> Text,
        password -> Text,
        email -> Text,
        verified -> Bool,
    }
}

#[derive(Queryable)]
pub struct User {
    pub id: i32,
    pub name: String,
    pub password: String,
    pub email: String,
    pub verified: bool,
}

#[insertable_into(users)]
pub struct NewUser<'a> {
    pub name: &'a str,
    pub password: &'a str,
    pub email: &'a str,
    pub verified: &'a bool,
}
