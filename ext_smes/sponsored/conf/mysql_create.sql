# Database create script for MySQL

# GENERAL TABLES 

## Subscribers table
CREATE TABLE subscribers (  
  address varchar(20) not null primary key,
  timezone varchar(50) not null,
  INDEX (timezone)
) ENGINE InnoDB;

## Distribution table
CREATE TABLE distributions (  
  name varchar(10) not null primary key,  
  status int not null default 0
) ENGINE InnoDB;

## Subscription table
CREATE TABLE subscriptions (  
  id int not null auto_increment primary key,
  subscriber_address varchar(20) not null,
  distribution_name varchar(10) not null,
  volume smallint not null,
  start_date timestamp not null default now(),
  end_date datetime,

  FOREIGN KEY (subscriber_address) REFERENCES subscribers(address),
  FOREIGN KEY (distribution_name) REFERENCES distributions(name),
  INDEX (subscriber_address, distribution_name)
) ENGINE InnoDB;

# TABLES SPECIAL FOR ADVERTISING DISTRIBUTION

CREATE TABLE deliveries (
  subscriber_address varchar(20) not null,  
  distribution_name varchar(10) not null,  
  start_date timestamp not null,
  send_date timestamp not null,
  end_date timestamp not null,                                      
  total smallint not null default 0,
  sended smallint not null default 0,
  timezone varchar(50) not null,
  PRIMARY KEY (subscriber_address, distribution_name),
  INDEX (send_date)
) ENGINE InnoDB;

## Delivery statistic
CREATE TABLE delivery_stats (
  subscriber_address varchar(20) not null,
  date date not null,
  delivered smallint not null,
  sended smallint not null,
  advertiser_id int not null,
  PRIMARY KEY (subscriber_address, date)
) ENGINE InnoDB;