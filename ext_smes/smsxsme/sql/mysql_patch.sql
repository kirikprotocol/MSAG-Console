CREATE INDEX sec_messages_index ON sec_messages(destination_address, status);

ALTER TABLE calendar_messages ADD (smpp_status INT);
#ALTER TABLE calendar_messages DROP delivery_status_id;

ALTER TABLE sec_messages ADD (smpp_status INT);
#ALTER TABLE sec_messages DROP delivery_status_id;

ALTER TABLE smpp_messages ADD status int;

CREATE TABLE sponsored_distribution_info (
  abonent CHAR(20) NOT NULL,
  today_cnt SMALLINT not null,
  cur_cnt SMALLINT not null,
  tm TIMESTAMP DEFAULT now()

  PRIMARY KEY(abonent),
  INDEX(today_cnt)
) CHARACTER SET koi8r;

CREATE TABLE sponsored_stats (
  address CHAR(20) NOT NULL,
  date DATE NOT NULL DEFAULT CURDATE(),
  cnt SMALLINT NOT NULL default 1,

  PRIMARY KEY (address, date)
);
