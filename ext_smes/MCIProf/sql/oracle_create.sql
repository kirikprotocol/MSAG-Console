create table results(
    abonent varchar2(12) not null,
    address varchar2(15) not null,
    scenario varchar2(32) not null,
    question integer not null,
    answer varchar2(255) not null,
    rdate integer not null
);
create index results_question on results (address, scenario, question);
create index results_rdate on results (rdate);

create sequence informmessages_seq START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE;

create table informmessages(
    id integer not null,
    encoding varchar2(20) not null,
    msg_eng blob not null,
    msg_rus blob not null,
    tm date not null,
    primary key(id)
);

create or replace trigger informmessages_trigger before 
 insert on informmessages for each row
begin
   select informmessages_seq.nextval into :new.id from dual;
end;
/

create table dispatch(
    started number(21) not null,
    ended number(21) not null
);

create table smppstats (
    rdate integer not null, 
    tm date not null, 
    direction char(1) not null, 
    abonentaddr varchar2(30),
    sourceaddr varchar2(30)
);
create index smppstats_rdate on smppstats (rdate,abonentaddr,sourceaddr);


create table sesstrack (
    rdate integer not null, 
    tm date not null, 
    abonent varchar2(12) not null,
    source varchar2(15) not null,
    scenario varchar2(32) default '',
    page varchar2(32) default '',
    data varchar2(32) default '',
    target varchar2(32) default '',
    error integer default 0
);
create index sesstrack_rdate on sesstrack (rdate,abonent,source);
