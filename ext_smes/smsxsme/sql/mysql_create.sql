## TABLES FOR CALENDAR
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
  msc_address VARCHAR(20),
  smpp_id BIGINT,
  adv int,

  PRIMARY KEY(id),
  INDEX(send_time)
) CHARACTER SET koi8r, ENGINE InnoDB;

 CREATE INDEX cld_msg_send_time USING BTREE ON calendar_messages(send_time);


## TABLES FOR SECRET
CREATE TABLE sec_users (
  address VARCHAR(30) NOT NULL,
  password TEXT NOT NULL,

  PRIMARY KEY(address)
) CHARACTER SET koi8r, ENGINE InnoDB;

CREATE TABLE sec_messages (
  id INT NOT NULL,
  destination_address VARCHAR(30) NOT NULL,
  source_address VARCHAR(30) NOT NULL,
  message TEXT,
  send_date TIMESTAMP NOT NULL DEFAULT now(),
  dest_addr_subunit INT NOT NULL,
  status INT NOT NULL,
  save_delivery_status INT NOT NULL,
  notify_originator INT NOT NULL,
  conn CHAR(10),
  smpp_status INT,
  msc_address VARCHAR(20),
  smpp_id BIGINT,
  adv int,

  PRIMARY KEY(id),
  INDEX(destination_address, status)
) CHARACTER SET koi8r ENGINE InnoDB;


-- TABLES FOR SENDER
CREATE TABLE smpp_messages (
  id INT NOT NULL,
  smpp_status INT,
  status INT NOT NULL,
  smpp_id BIGINT,

  PRIMARY KEY(id),
  INDEX(smpp_id)
) CHARACTER SET koi8r, ENGINE InnoDB;


##TABLES FOR NICKS
CREATE TABLE nicks (
  address CHAR(20) NOT NULL,
  nick CHAR(20) NOT NULL,

  PRIMARY KEY(address)
) ENGINE InnoDB;

##TABLES FOR GROUP
CREATE TABLE group_edit_profiles (
  address CHAR(20) NOT NULL PRIMARY KEY,
  send_notifications SMALLINT NOT NULL,
  lock_group_edit SMALLINT NOT NULL
) ENGINE InnoDB;

CREATE TABLE group_edit_lists (
  id INT NOT NULL PRIMARY KEY,
  name VARCHAR(200) NOT NULL,
  owner CHAR(20) NOT NULL,
  max_elements INT NOT NULL,

  UNIQUE INDEX (name, owner)
) CHARACTER SET koi8r, ENGINE InnoDB;

CREATE TABLE group_edit_members (
  group_id INT NOT NULL,
  address CHAR(20) NOT NULL,

  INDEX (group_id),
  UNIQUE INDEX (group_id, address)
) ENGINE InnoDB;

CREATE TABLE group_edit_submitters (
  group_id INT NOT NULL,
  address CHAR(20) NOT NULL,

  INDEX (group_id),
  UNIQUE INDEX (group_id, address)
) ENGINE InnoDB;

CREATE TABLE group_edit_principals (
  address CHAR(20) NOT NULL PRIMARY KEY,
  max_lists INT NOT NULL,
  max_members_per_list INT NOT NULL
) ENGINE InnoDB;

CREATE TABLE group_send (
  id INT NOT NULL,
  address VARCHAR(20) NOT NULL,
  smpp_id BIGINT,
  status TINYINT NOT NULL DEFAULT 0,

  INDEX(id),
  INDEX (smpp_id)
) ENGINE InnoDB;

##TABLES FOR BLACK LIST
CREATE TABLE black_list (
  msisdn CHAR(20) NOT NULL PRIMARY KEY
) ENGINE InnoDB;
