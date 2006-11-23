CREATE TABLE messages2send (
  id int not null auto_increment,
  abonent varchar(30) not null,
  message text not null,
  to_abonent varchar(30) not null,
  time timestamp not null default now(),
  send_time timestamp not null,

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

  primary key(id)
) CHARACTER SET koi8r;

