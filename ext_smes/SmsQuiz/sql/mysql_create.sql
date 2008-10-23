

create table smsquiz_subscriptions
(
    address varchar(32) not null primary key ,
    start_date DATETIME not null,
    end_date DATETIME
) engine=InnoDB character set utf8;


