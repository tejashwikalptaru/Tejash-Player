CREATE TABLE `keys` (
   `id` int UNIQUE NOT NULL AUTO_INCREMENT,
   `serial` varchar(100),
   `name` varchar(200),
   `email` varchar(100),
   `date_reg` varchar(100),
   PRIMARY KEY(id)
);