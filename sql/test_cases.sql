/* Message store */
--Восстановление конектов к БД после alter system kill session
select sid, serial#, status from v$session where machine='smsc' and osuser='bryz';
select 'alter system kill session ''' || sid || ',' || serial# || ''';' from v$session where machine='smsc' and osuser='bryz' and status != 'KILLED'
alter system kill session '13,8118';


--Работа с БД с большим размером пула соединений
select count(*) from v$session where machine='smsc' and osuser='bryz';


--Все коннекты к БД одинаково равномерно загружены
select stat.statistic#, nam.name, max(stat.value), min(stat.value) from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# in (4,5,6) and
ses.machine='smsc' and ses.osuser='bryz'
group by stat.statistic#, nam.name

select stat.statistic#, nam.name, stat.value from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# = 4 and
ses.machine='smsc' and ses.osuser='bryz'
order by stat.value


--Поведение Message Store при временной блокировке всей таблицы
lock table sms_msg in exclusive mode
commit


/* Проверка целостности данных после архиватора */
--Процесс работы архиватора
select (select count(*) from smsc.sms_msg where st != 0) msg2arc, (select count(*) from smsc.sms_arc) arc from dual


--Все сообщения с финальным статусом переносятся из sms_msg в архив
select count(*) from smsc.sms_msg where st != 0


--В архиве сообщения только с финальном статусом
select count(*) from smsc.sms_arc where st = 0


--Записи в sms_msg и sms_arc не пересекаются
select count(*) from smsc.sms_msg msg, smsc.sms_arc arc where msg.id = arc.id


--Записи sms_br в точности соответствуют записям в sms_arc
select (select count(*) from smsc.sms_arc) arc, (select count(*) from cboss.sms_br) br, (select count(*) from smsc.sms_arc arc, cboss.sms_br br where arc.id  = br.id) intersection from dual


--Записи не теряются при переносе в sms_arc
select (select max(id) from smsc.sms_msg) max_msg, (select max(id) from smsc.sms_arc) max_arc, ((select count(*) from smsc.sms_msg) + (select count(*) from smsc.sms_arc)) total from dual

--Записи из sms_msg выгребаются в sms_arc упорядоченно по времени перевода в финальное состояние


/* Триггеры для ломания транзакций в архиваторе */
/* smsc */
drop trigger smsc.smsc_msg_delete;
drop trigger smsc.smsc_arc_insert;
drop procedure smsc.tx_check;
drop sequence smsc.tx_test_seq;
drop table smsc.tx_test;

create table smsc.tx_test
(
	table_name varchar2(10),
	counter integer,
	seq_number integer
);
insert into smsc.tx_test(table_name, counter) values('sms_msg', 0);
insert into smsc.tx_test(table_name, counter) values('sms_arc', 0);

create sequence smsc.tx_test_seq increment by 1 start with 1 nomaxvalue order;

create or replace procedure smsc.tx_check(tbl_name varchar2) is
	cnt number;
	old_seq_num number;
    new_seq_num number;
begin
	select counter, seq_number into cnt, old_seq_num from tx_test where table_name = tbl_name;
	if cnt > 0 then
		select tx_test_seq.nextval into new_seq_num from dual;
		cnt := cnt - 1;
		update tx_test set counter = cnt, seq_number = new_seq_num where table_name = tbl_name;
		if cnt = 0 and (new_seq_num - old_seq_num) = 1 then
			raise VALUE_ERROR;
		end if;
	end if;
exception when NO_DATA_FOUND then
	return;
end;
/

create or replace trigger smsc.smsc_msg_delete before delete on smsc.sms_msg call tx_check('sms_msg')
/

create or replace trigger smsc.smsc_arc_insert before insert on smsc.sms_arc call tx_check('sms_arc')
/

/* cboss */
drop trigger cboss.smsc_br_insert;
drop sequence cboss.tx_test_seq;
drop table cboss.tx_test;

create table cboss.tx_test
(
	table_name varchar2(10),
	counter integer,
	seq_number integer
);
insert into cboss.tx_test(table_name, counter) values('sms_br', 0);

create sequence cboss.tx_test_seq increment by 1 start with 1 nomaxvalue order;

create or replace procedure cboss.tx_check(tbl_name varchar2) is
	cnt number;
	old_seq_num number;
    new_seq_num number;
begin
	select counter, seq_number into cnt, old_seq_num from tx_test where table_name = tbl_name;
	if cnt > 0 then
		select tx_test_seq.nextval into new_seq_num from dual;
		cnt := cnt - 1;
		update tx_test set counter = cnt, seq_number = new_seq_num where table_name = tbl_name;
		if cnt = 0 and (new_seq_num - old_seq_num) = 1 then
			raise VALUE_ERROR;
		end if;
	end if;
exception when NO_DATA_FOUND then
	return;
end;
/

create or replace trigger cboss.smsc_br_insert before insert on cboss.sms_br for each row
call tx_check('sms_br')
/
