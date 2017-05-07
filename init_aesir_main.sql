CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    username citext NOT NULL,
    password CHARACTER(60) NOT NULL,
    email citext NOT NULL,
    login_attempts SMALLINT NOT NULL DEFAULT 0,
    verification_code text DEFAULT NULL,
    admin SMALLINT NOT NULL DEFAULT 0
);

CREATE TABLE licenses (
    id BIGSERIAL PRIMARY KEY,
    "name" text NOT NULL,
    author text NOT NULL,
    license text NOT NULL
);

CREATE TABLE banned_users (
    id BIGSERIAL PRIMARY KEY,
    ip text NULL,
    user_id BIGINT NULL
);

CREATE TABLE settings (
    "name" text NOT NULL,
    value text NOT NULL
);

ALTER TABLE users ADD CONSTRAINT "users_username_unique" UNIQUE (username);
ALTER TABLE banned_users ADD CONSTRAINT "banned_users_user_id_fkey" FOREIGN KEY (user_id) REFERENCES users(id);
