CREATE TABLE base_stats (
    id BIGSERIAL PRIMARY KEY,
    str INTEGER NOT NULL,
    agi INTEGER NOT NULL,
    wis INTEGER NOT NULL,
    hp INTEGER NOT NULL,
    exp_curve INTEGER NOT NULL,
    max_level INTEGER NOT NULL
);

CREATE TABLE stats (
    id BIGSERIAL PRIMARY KEY,
    str INTEGER NOT NULL,
    agi INTEGER NOT NULL,
    wis INTEGER NOT NULL,
    hp INTEGER NOT NULL,
    exp INTEGER NOT NULL,
    level INTEGER NOT NULL
);

CREATE TABLE locations (
    id BIGSERIAL PRIMARY KEY,
    zone_id BIGINT NOT NULL,
    x INTEGER NOT NULL,
    y INTEGER NOT NULL
);

CREATE TABLE zones (
    id BIGSERIAL PRIMARY KEY,
    "name" text NOT NULL,
    data text NOT NULL
);

CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    username citext NOT NULL,
    password CHARACTER(60) NOT NULL,
    email citext NOT NULL,
    login_attempts SMALLINT NOT NULL DEFAULT 0,
    verification_code text DEFAULT NULL
);

CREATE TABLE players (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    base_stats_id BIGINT NOT NULL,
    stats_id BIGINT NOT NULL,
    "name" citext NOT NULL
);

CREATE TABLE items (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    base_stats_id BIGINT NOT NULL,
    stats_id BIGINT NOT NULL,
    "name" citext NOT NULL
);

CREATE TABLE gods (
    id BIGSERIAL PRIMARY KEY,
    "name" text NOT NULL
);

CREATE TABLE monsters (
    id BIGSERIAL PRIMARY KEY,
    location_id BIGINT NOT NULL,
    stats_id BIGINT NOT NULL,
    "name" citext NOT NULL
);

CREATE TABLE licenses (
    id BIGSERIAL PRIMARY KEY,
    "name" text NOT NULL,
    author text NOT NULL,
    license text NOT NULL
);

ALTER TABLE locations ADD CONSTRAINT "locations_map_id_fkey" FOREIGN KEY (zone_id) REFERENCES zones(id);
ALTER TABLE players ADD CONSTRAINT "players_base_stats_id_fkey" FOREIGN KEY (base_stats_id) REFERENCES base_stats(id);
ALTER TABLE players ADD CONSTRAINT "players_stats_id_fkey" FOREIGN KEY (stats_id) REFERENCES stats(id);
ALTER TABLE players ADD CONSTRAINT "players_user_id_fkey" FOREIGN KEY (user_id) REFERENCES users(id);
ALTER TABLE items ADD CONSTRAINT "items_base_stats_id_fkey" FOREIGN KEY (base_stats_id) REFERENCES base_stats(id);
ALTER TABLE items ADD CONSTRAINT "items_stats_id_fkey" FOREIGN KEY (stats_id) REFERENCES stats(id);
ALTER TABLE items ADD CONSTRAINT "items_user_id_fkey" FOREIGN KEY (user_id) REFERENCES users(id);
ALTER TABLE monsters ADD CONSTRAINT "monsters_location_id_fkey" FOREIGN KEY (location_id) REFERENCES locations(id);
ALTER TABLE monsters ADD CONSTRAINT "monsters_stats_id_fkey" FOREIGN KEY (stats_id) REFERENCES stats(id);

