table! {
    locations {
        id -> Integer,
        map_id -> Integer,
        x -> Integer,
        y -> Integer,
    }
}

#[derive(Queryable)]
pub struct Location {
    pub id: i32,
    pub map_id: i32,
    pub x: i32,
    pub y: i32,
}

#[insertable_into(locations)]
pub struct NewLocation {
    pub map_id: i32,
    pub x: i32,
    pub y: i32,
}
