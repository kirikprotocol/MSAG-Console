/* Message store */
--�������������� �������� � �� ����� alter system kill session
select sid, serial#, status from v$session where machine='smsc' and osuser='bryz';
select 'alter system kill session ''' || sid || ',' || serial# || ''';' from v$session where machine='smsc' and osuser='bryz' and status != 'KILLED'
alter system kill session '13,8118';


--������ � �� � ������� �������� ���� ����������
select count(*) from v$session where machine='smsc' and osuser='bryz';


--��� �������� � �� ��������� ���������� ���������
select stat.statistic#, nam.name, max(stat.value), min(stat.value) from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# in (4,5,6) and
ses.machine='smsc' and ses.osuser='bryz'
group by stat.statistic#, nam.name

select stat.statistic#, nam.name, stat.value from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# = 4 and
ses.machine='smsc' and ses.osuser='bryz'
order by stat.value


--��������� Message Store ��� ��������� ���������� ���� �������
lock table sms_msg in exclusive mode
commit


/* �������� ����������� ������ ����� ���������� */
--������� ������ ����������
select (select count(*) from smsc.sms_msg where st != 0) msg2arc, (select count(*) from smsc.sms_arc) arc from dual


--��� ��������� � ��������� �������� ����������� �� sms_msg � �����
select count(*) from smsc.sms_msg where st != 0


--� ������ ��������� ������ � ��������� ��������
select count(*) from smsc.sms_arc where st = 0


--������ � sms_msg � sms_arc �� ������������
select count(*) from smsc.sms_msg msg, smsc.sms_arc arc where msg.id = arc.id


--������ sms_br � �������� ������������� ������� � sms_arc
select (select count(*) from smsc.sms_arc) arc, (select count(*) from cboss.sms_br) br, (select count(*) from smsc.sms_arc arc, cboss.sms_br br where arc.id  = br.id) intersection from dual


--������ �� �������� ��� �������� � sms_arc
select (select result from smsc.test_result where table_name = 'sms_msg') expected, ((select count(*) from smsc.sms_msg) + (select count(*) from smsc.sms_arc)) real from dual

--������ �� sms_msg ����������� � sms_arc ������������ �� ������� �������� � ��������� ���������

/* ������� ��� �������� insert � sms_msg */
create table smsc.test_result
(
	table_name varchar2(10),
	result integer
);
insert into smsc.test_result(table_name, result) values('sms_msg', 0);

create or replace trigger smsc.smsc_msg_insert after insert on smsc.sms_msg for each row
update test_result set result = result + 1 where table_name = 'sms_msg'
/


/* �������� ��� ������� ���������� � ���������� */
select cboss.tx_test_seq.nextval from dual;
delete from cboss.tx_test;
insert into cboss.tx_test(table_name, seq_number) values ('sms_br', 2300);

select smsc.tx_test_seq.nextval from dual;
delete from smsc.tx_test;
insert into smsc.tx_test(table_name, seq_number) values ('sms_msg', 1);
insert into smsc.tx_test(table_name, seq_number) values ('sms_arc', 1);

/* smsc */
drop trigger smsc.smsc_msg_delete;
drop trigger smsc.smsc_arc_insert;
drop procedure smsc.tx_check;
drop sequence smsc.tx_test_seq;
drop table smsc.tx_test;

create table smsc.tx_test
(
	table_name varchar2(10),
	seq_number integer
);

create sequence smsc.tx_test_seq increment by 1 start with 1 nomaxvalue order;

create or replace procedure smsc.tx_check(tbl_name varchar2) is
	seq_num number;
begin
	select tx_test_seq.nextval into seq_num from dual;
	for rec in (select seq_number from tx_test where table_name = tbl_name) loop
		if rec.seq_number = seq_num then
			raise VALUE_ERROR;
		end if;
	end loop;
end;
/

create or replace trigger smsc.smsc_msg_delete before delete on smsc.sms_msg for each row
call tx_check('sms_msg')
/

create or replace trigger smsc.smsc_arc_insert before insert on smsc.sms_arc for each row
call tx_check('sms_arc')
/

/* cboss */
drop trigger cboss.smsc_br_insert;
drop sequence cboss.tx_test_seq;
drop table cboss.tx_test;

create table cboss.tx_test
(
	table_name varchar2(10),
	seq_number integer
);

create sequence cboss.tx_test_seq increment by 1 start with 1 nomaxvalue order;

create or replace procedure cboss.tx_check(tbl_name varchar2) is
	seq_num number;
begin
	select tx_test_seq.nextval into seq_num from dual;
	for rec in (select seq_number from tx_test where table_name = tbl_name) loop
		if rec.seq_number = seq_num then
			raise VALUE_ERROR;
		end if;
	end loop;
end;
/

create or replace trigger cboss.smsc_br_insert before insert on cboss.sms_br for each row
call tx_check('sms_br')
/
