

create table smsquiz_subscriptions
(
    address varchar(32) not null primary key ,
    start_date timestamp not null,
    end_date timestamp
) engine=InnoDB character set utf8;


