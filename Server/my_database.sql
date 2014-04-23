-- create the events DB
DROP DATABASE IF EXISTS plantprotector;
CREATE DATABASE plantprotector;
USE plantprotector;

CREATE USER "bbb_user"@"%" IDENTIFIED BY "bbb_password";
GRANT ALL ON plantprotector.* to "bbb_user"@"%";


CREATE TABLE plant_data(temperature INT NOT NULL, humidity INT NOT NULL, light INT NOT NULL, unixtime TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP); 
