CREATE TABLE subscribed_abonents (
  abonent_address varchar(30) not null,
  register_time timestamp not null default "now()",

  primary key(abonent_address)
);

CREATE INDEX subscribed_abonents_index ON subscribed_abonents(abonent_address);
