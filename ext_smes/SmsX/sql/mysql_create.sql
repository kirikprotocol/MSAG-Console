CREATE TABLE messages2send (
  id int not null auto_increment,
  abonent varchar(30) not null,
  message text not null,
  to_abonent varchar(30) not null,
  time timestamp not null default now(),
  send_time timestamp not null,
  dest_addr_subunit int not null,

  primary key(id),
  index(send_time)
) CHARACTER SET koi8r;

CREATE TABLE secret_users (
  number varchar(30) not null,
  password text not null,

  primary key(number)
) CHARACTER SET koi8r;

CREATE TABLE secret_messages (
  id int not null auto_increment,
  user_number varchar(30) not null,
  from_number varchar(30) not null,
  message text,
  send_date timestamp not null default now(),
  dest_addr_subunit int not null,

  primary key(id)
) CHARACTER SET koi8r;

create table smppstats (
    rdate int not null,
    tm timestamp not null,
    direction char(1) not null,
    abonentaddr varchar(30),
    sourceaddr varchar(30),
    message text,
    conn_name varchar(32),
    operator varchar(50),
    ussdop int not null default -1
);

create index smppstats_rdate on smppstats (rdate,abonentaddr);
create index smppstats_conn_name on smppstats (conn_name,abonentaddr);
create index smppstats_operator on smppstats (operator,abonentaddr);
create index smppstats_ras on smppstats (rdate,abonentaddr,sourceaddr);


CREATE TABLE sponsored (
  abonent VARCHAR (20) NOT NULL PRIMARY KEY,
  cnt INTEGER NOT NULL,
  tm TIMESTAMP DEFAULT now()
);

CREATE TABLE advertstats (
  abonent VARCHAR (20) NOT NULL,
  msg_id INTEGER,
  delivery_id INTEGER,
  tm TIMESTAMP DEFAULT now(),
  status INTEGER
);

CREATE TABLE advertmsg (
  id INTEGER UNSIGNED PRIMARY KEY AUTO_INCREMENT,
  msg VARCHAR (160) NOT NULL,
  tm TIMESTAMP DEFAULT now()
);

CREATE TABLE delivery (
  id INTEGER UNSIGNED PRIMARY KEY AUTO_INCREMENT,
  msg_id INTEGER NOT NULL,
  msg VARCHAR (610) NOT NULL,
  abonent VARCHAR (20) NOT NULL,
  status INTEGER
) CHARACTER SET koi8r;



