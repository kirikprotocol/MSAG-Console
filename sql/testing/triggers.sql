/*
	Тест кейсы:
	 - В архиве сообщения только с финальным статусом
	 - Записи не теряются при переносе из sms_msg в sms_arc
	 - Данные не искажаются при переносе из sms_msg в sms_arc
	 - Все сообщения с финальным статусом переносятся из sms_msg в архив
	 - Только сообщения с установленным признаком архивации переносятся в архив
	 
	Эмуляция rollback по секундам:
	                | 00 | 10 | 20 | 30 | 40 | 50 |
	+---------------+----+----+----+----+----+----+
	 sms_br	insert  |  x |  x |    |    |  x |  x |
	 sms_msg delete |  x |    |  x |    |  x |    |
	 sms_arc insert |  x |    |    |  x |    |  x |
	 
	 Коды ошибок:
	 -20101 - эмуляция rollback
	 -20201 - ошибки переноса в sms_arc
*/

/* Таблица результатов */
drop table results;
create table results
(
   	tbl varchar2(10),
	msg_type varchar2(10),
	action varchar2(10),
	value integer default 0
);
create unique index results_pk on results (tbl, msg_type, action);
insert into results(tbl, msg_type, action) values ('sms_msg', 'arc', 'insert');
insert into results(tbl, msg_type, action) values ('sms_msg', 'total', 'insert');
insert into results(tbl, msg_type, action) values ('sms_msg', 'arc', 'delete');
insert into results(tbl, msg_type, action) values ('sms_msg', 'total', 'delete');
insert into results(tbl, msg_type, action) values ('sms_arc', null, 'insert');
insert into results(tbl, msg_type, action) values ('sms_br', null, 'insert');
--insert into results(tbl, msg_type, action) values ('sms_msg', null, 'rollback');
--insert into results(tbl, msg_type, action) values ('sms_arc', null, 'rollback');
--insert into results(tbl, msg_type, action) values ('sms_br', null, 'rollback');

/* Триггер для эмуляции rollback и проверки правильности переноса записей sms_msg -> sms_arc */
create or replace trigger smsc_arc_insert after insert on sms_arc
	referencing new as arc for each row
declare
	flag number;
	sec char(2);
	msg sms_msg%rowtype;
begin
	--emulate rollback
	select count(*) into flag from results
		where tbl = 'sms_arc' and action = 'rollback';
	if flag > 0 then
		select to_char(sysdate, 'ss') into sec from dual;
		if sec = '00' or sec = '30' or sec = '50' then
			raise_application_error(-20101, 'Emulating sms_arc insert rollback');
		end if;
	end if;
	--update statistics
	update results set value = value + 1
		where tbl = 'sms_arc' and action = 'insert';
	--check
	begin
		--state
		if :arc.st = 0 then
			raise_application_error(-20201, 'arc.st = ENROTE');
		end if;
		--arc = 'Y'
		select * into msg from sms_msg where id = :arc.id;
		if msg.arc != 'Y' then
			raise_application_error(-20201, 'Trying to archivate sms with arc != ''Y''');
		end if;
		--data
		if msg.st != :arc.st then
			raise_application_error(-20201, 'msg.st != arc.st');
		end if;
		if msg.mr != :arc.mr then
			raise_application_error(-20201, 'msg.mr != arc.mr');
		end if;
		if msg.oa_ton != :arc.oa_ton then
			raise_application_error(-20201, 'msg.oa_ton != arc.oa_ton');
		end if;
		if msg.oa_npi != :arc.oa_npi then
			raise_application_error(-20201, 'msg.oa_npi != arc.oa_npi');
		end if;
		if msg.oa_val != :arc.oa_val then
			raise_application_error(-20201, 'msg.oa_val != arc.oa_val');
		end if;
		if msg.src_msc != :arc.src_msc then
			raise_application_error(-20201, 'msg.src_msc != arc.src_msc');
		end if;
		if msg.src_imsi != :arc.src_imsi then
			raise_application_error(-20201, 'msg.src_imsi != arc.src_imsi');
		end if;
		if msg.src_sme_n != :arc.src_sme_n then
			raise_application_error(-20201, 'msg.src_sme_n != arc.src_sme_n');
		end if;
		if msg.da_ton != :arc.da_ton then
			raise_application_error(-20201, 'msg.da_ton != arc.da_ton');
		end if;
		if msg.da_npi != :arc.da_npi then
			raise_application_error(-20201, 'msg.da_npi != arc.da_npi');
		end if;
		if msg.da_val != :arc.da_val then
			raise_application_error(-20201, 'msg.da_val != arc.da_val');
		end if;
		if msg.dst_msc != :arc.dst_msc then
			raise_application_error(-20201, 'msg.dst_msc != arc.dst_msc');
		end if;
		if msg.dst_imsi != :arc.dst_imsi then
			raise_application_error(-20201, 'msg.dst_imsi != arc.dst_imsi');
		end if;
		if msg.dst_sme_n != :arc.dst_sme_n then
			raise_application_error(-20201, 'msg.dst_sme_n != arc.dst_sme_n');
		end if;
		if msg.wait_time != :arc.wait_time then
			raise_application_error(-20201, 'msg.wait_time != arc.wait_time');
		end if;
		if msg.valid_time != :arc.valid_time then
			raise_application_error(-20201, 'msg.valid_time != arc.valid_time');
		end if;
		if msg.submit_time != :arc.submit_time then
			raise_application_error(-20201, 'msg.submit_time != arc.submit_time');
		end if;
		if msg.attempts != :arc.attempts then
			raise_application_error(-20201, 'msg.attempts != arc.attempts');
		end if;
		if msg.last_result != :arc.last_result then
			raise_application_error(-20201, 'msg.last_result != arc.last_result');
		end if;
		if msg.last_try_time != :arc.last_try_time then
			raise_application_error(-20201, 'msg.last_try_time != arc.last_try_time');
		end if;
		if msg.dr != :arc.dr then
			raise_application_error(-20201, 'msg.dr != arc.dr');
		end if;
		if msg.pri != :arc.pri then
			raise_application_error(-20201, 'msg.pri != arc.pri');
		end if;
		if msg.pid != :arc.pid then
			raise_application_error(-20201, 'msg.pid != arc.pid');
		end if;
		if msg.udhi != :arc.udhi then
			raise_application_error(-20201, 'msg.udhi != arc.udhi');
		end if;
		if msg.dcs != :arc.dcs then
			raise_application_error(-20201, 'msg.dcs != arc.dcs');
		end if;
		if msg.udl != :arc.udl then
			raise_application_error(-20201, 'msg.udl != arc.udl');
		end if;
		if msg.ud != :arc.ud then
			raise_application_error(-20201, 'msg.ud != arc.ud');
		end if;
		if msg.rcpt_id != :arc.rcpt_id then
			raise_application_error(-20201, 'msg.rcpt_id != arc.rcpt_id');
		end if;
		if msg.esm_class != :arc.esm_class then
			raise_application_error(-20201, 'msg.esm_class != arc.esm_class');
		end if;
	exception
		when no_data_found then
			raise_application_error(-20201, 'No sms_msg record found for id = ' || :arc.id);
	end;
end;
/

/* Триггер для подсчета insert в sms_msg */
create or replace trigger smsc_msg_insert after insert on sms_msg
	referencing new as msg for each row
begin
	if :msg.arc = 'Y' then
		update results set value = value + 1
			where tbl = 'sms_msg' and msg_type = 'arc' and action = 'insert';
	end if;
	update results set value = value + 1
		where tbl = 'sms_msg' and msg_type = 'total' and action = 'insert';
end;
/

/* Триггер для эмуляции rollback и подсчета и проверки delete в sms_msg */
create or replace trigger sms_msg_delete before delete on sms_msg
	referencing old as msg for each row
declare
	flag number;
	sec char(2);
begin
	--emulate rollback
	select count(*) into flag from results
		where tbl = 'sms_msg' and action = 'rollback';
	if flag > 0 then
        select to_char(sysdate, 'ss') into sec from dual;
		if sec = '00' or sec = '20' or sec = '40' then
			raise_application_error(-20101, 'Emulating sms_msg delete rollback');
		end if;
	end if;
	--check & update statistics
	if :msg.arc = 'Y' then
		select count(*) into flag from sms_arc where id = :msg.id;
		if flag = 0 then
			raise_application_error(-20101, 'Trying to delete arc record in sms_msg that has no corresponding sms_arc record');
		end if;
		update results set value = value + 1
			where tbl = 'sms_msg' and msg_type = 'arc' and action = 'delete';
	end if;
	update results set value = value + 1
		where tbl = 'sms_msg' and msg_type = 'total' and action = 'delete';
end;
/

/* Триггер для для эмуляции rollback и подсчета insert в sms_br */
create or replace trigger sms_br_insert after insert on sms_br
	referencing new as br for each row
declare
	flag number;
	sec char(2);
begin
	--emulate rollback
	select count(*) into flag from results
		where tbl = 'sms_br' and action = 'rollback';
	if flag > 0 then
        select to_char(sysdate, 'ss') into sec from dual;
		if sec = '00' or sec = '10' or sec = '40' or sec = '50' then
			raise_application_error(-20101, 'Emulating sms_br insert rollback');
		end if;
	end if;
	--update statistics
	update results set value = value + 1
		where tbl = 'sms_br' and action = 'insert';
end;
/

