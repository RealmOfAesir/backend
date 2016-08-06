table! {
    zones {
        id -> Integer,
        name -> Text,
        data -> Text,
    }
}

#[derive(Queryable)]
pub struct Zone {
    pub id: i32,
    pub name: String,
    pub data: String,
}

#[insertable_into(zones)]
pub struct NewZone<'a> {
    pub name: &'a str,
    pub data: &'a str,
}
