INSERT INTO licenses ("name", author, license) VALUES ('http://pousse.rapiere.free.fr/tome/tome-tiles.htm',  'David Gervais', 'https://creativecommons.org/licenses/by/3.0/');

--INSERT INTO gods ("name") VALUES ('Andhrimnir');
--INSERT INTO gods ("name") VALUES ('Loki');
--INSERT INTO gods ("name") VALUES ('Odin');
--INSERT INTO gods ("name") VALUES ('Thor');
--INSERT INTO gods ("name") VALUES ('Freya');

INSERT INTO settings ("name", value) VALUES ('maintenance_mode', '0');

INSERTO INTO users (username, password, email, admin) VALUES ('admin', '$argon2i$v=19$m=131072,t=6,p=1$Bckylh6Uzq+jGwImc++45Q$/gERM28ywUHBxkleHeahMcq5PyEtsbx840RxCprLk+s', 'realm.of.aesir@gmail.com', 1);

