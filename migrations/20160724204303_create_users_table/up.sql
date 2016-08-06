CREATE TABLE zones (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    data TEXT NOT NULL
);

CREATE TABLE locations (
    id SERIAL PRIMARY KEY,
    map_id INT REFERENCES zones(id) NOT NULL,
    x INT NOT NULL,
    y INT NOT NULL
);

CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    location_id INT REFERENCES locations(id),
    name CITEXT NOT NULL,
    password CHAR(60) NOT NULL,
    email CITEXT NOT NULL,
    verification_code TEXT NULL
);

CREATE TABLE stats (
    id SERIAL PRIMARY KEY,
    str INT NOT NULL,
    agi INT NOT NULL,
    wis INT NOT NULL,
    hp INT NOT NULL,
    exp INT NOT NULL,
    level INT NOT NULL
);

CREATE TABLE base_stats (
    id SERIAL PRIMARY KEY,
    str INT NOT NULL,
    agi INT NOT NULL,
    wis INT NOT NULL,
    hp INT NOT NULL,
    exp_curve INT NOT NULL,
    max_level INT NULL
);

CREATE TABLE players (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id) NOT NULL,
    base_stats_id INT REFERENCES base_stats(id) NOT NULL,
    stats_id INT REFERENCES stats(id) NOT NULL,
    name CITEXT NOT NULL
);

CREATE TABLE items (
    id SERIAL PRIMARY KEY,
    user_id INT REFERENCES users(id) NOT NULL,
    base_stats_id INT REFERENCES base_stats(id) NOT NULL,
    stats_id INT REFERENCES stats(id) NOT NULL,
    name CITEXT NOT NULL
);

CREATE TABLE monsters (
    id SERIAL PRIMARY KEY,
    location_id INT REFERENCES locations(id),
    stats_id INT REFERENCES stats(id) NOT NULL,
    name CITEXT NOT NULL
);

CREATE TABLE gods (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE licenses (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    author TEXT NOT NULL,
    license TEXT NOT NULL
);

INSERT INTO zones (name, data) VALUES ('PlayerStart', '{ "height":10,
 "layers":[
        {
         "compression":"zlib",
         "data":"eJzjYmBg4KIBNidSjFTMAsXUUjdQ\/sCFAZHlBIU=",
         "encoding":"base64",
         "height":10,
         "name":"Tile Layer 1",
         "opacity":1,
         "type":"tilelayer",
         "visible":true,
         "width":10,
         "x":0,
         "y":0
        }],
 "nextobjectid":1,
 "orientation":"orthogonal",
 "renderorder":"right-down",
 "tileheight":32,
 "tilesets":[
        {
         "columns":9,
         "firstgid":1,
         "image":"dg_grounds32.gif",
         "imageheight":608,
         "imagewidth":288,
         "margin":0,
         "name":"base",
         "spacing":0,
         "tilecount":171,
         "tileheight":32,
         "tilewidth":32
        }],
 "tilewidth":32,
 "version":1,
 "width":10
}');
INSERT INTO gods (name) VALUES ('Andhrimnir');
INSERT INTO gods (name) VALUES ('Loki');
INSERT INTO gods (name) VALUES ('Odin');
INSERT INTO gods (name) VALUES ('Thor');
INSERT INTO gods (name) VALUES ('Freya');
INSERT INTO licenses (name, author, license) VALUES ('http://pousse.rapiere.free.fr/tome/tome-tiles.htm', 'David Gervais', 'https://creativecommons.org/licenses/by/3.0/');
