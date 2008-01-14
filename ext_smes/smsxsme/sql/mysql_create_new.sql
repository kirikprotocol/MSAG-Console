# TABLES FOR CALENDAR
CREATE TABLE calendar_messages (
  id INT NOT NULL,
  source_address VARCHAR(30) NOT NULL,
  destination_address VARCHAR(30) NOT NULL,
  send_time TIMESTAMP NOT NULL DEFAULT NOW(),
  dest_addr_subunit INT NOT NULL,
  status INT NOT NULL,
  save_delivery_status INT NOT NULL,
  message TEXT NOT NULL,
  smpp_status INT,
  conn CHAR(10),

  PRIMARY KEY(id),
  INDEX(send_time)
) CHARACTER SET koi8r;


# TABLES FOR SECRET
CREATE TABLE sec_users (
  address VARCHAR(30) NOT NULL,
  password TEXT NOT NULL,

  PRIMARY KEY(address)
) CHARACTER SET koi8r;

CREATE TABLE sec_messages (
  id INT NOT NULL,
  destination_address VARCHAR(30) NOT NULL,
  source_address VARCHAR(30) NOT NULL,
  message TEXT,
  send_date TIMESTAMP NOT NULL DEFAULT NOW(),
  dest_addr_subunit INT NOT NULL,
  status INT NOT NULL,
  save_delivery_status INT NOT NULL,
  notify_originator INT NOT NULL,
  conn CHAR(10),

  PRIMARY KEY(id),
  INDEX(destination_address, status)
) CHARACTER SET koi8r;


# TABLES FOR SENDER
CREATE TABLE smpp_messages (
  id INT NOT NULL,
  smpp_status INT,

  PRIMARY KEY(id)
) CHARACTER SET koi8r;


#TABLES FOR SPONSORED
CREATE TABLE sponsored_distribution_info (
  abonent CHAR(20) NOT NULL,
  cnt SMALLINT NOT NULL DEFAULT 0,
  today_cnt SMALLINT NOT NULL DEFAULT 0,
  cur_cnt SMALLINT NOT NULL DEFAULT 0,
  tm TIMESTAMP DEFAULT now(),

  PRIMARY KEY(abonent),
  INDEX(today_cnt)
) CHARACTER SET koi8r;

CREATE TABLE sponsored_delivery_stats (
  address CHAR(20) NOT NULL,
  date DATE NOT NULL,
  cnt SMALLINT NOT NULL default 1,

  PRIMARY KEY (address, date)
);


#TABLES FOR NICKS
CREATE TABLE nicks (
  address CHAR(20) NOT NULL,
  nick CHAR(20) NOT NULL,

  PRIMARY KEY(address)
);