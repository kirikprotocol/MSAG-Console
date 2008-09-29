

create table smsquiz_subscriptions
(
    address integer not null primary key ,
    start_date timestamp not null,
    end_date timestamp
) engine=InnoDB character set utf8;


