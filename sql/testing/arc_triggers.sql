/*
	Тест кейсы:
	 - В архиве сообщения только с финальным статусом
	 - Записи не теряются при переносе из sms_msg в sms_arc
	 - Данные не искажаются при переносе из sms_msg в sms_arc
	 - Все сообщения с финальным статусом переносятся из sms_msg в архив
	 - Только сообщения с установленным признаком архивации переносятся в архив
*/

drop table arc_stat;
create table arc_stat
(
	final_rec integer,
	last_arc date
) tablespace smsc_data;
insert into arc_stat (final_rec, last_arc) values (0, sysdate + 1/24);

--проверка условий на активацию архиватора
create or replace procedure check_arc_stat is
	final_max integer := 300; --config.xml: MessageStore/Archive/finalized
    interval_max integer := 40; --config.xml: MessageStore/Archive/interval
	stat arc_stat%rowtype;
	interval integer;
begin
	select * into stat from arc_stat;
	interval := sysdate - stat.last_arc;
	--архиватор стартует не реже, чем прописано в конфиге
	if stat.final_rec > 1.2 * final_max then --20% допуск (побольше для cancel_sm)
		raise_application_error(-20201, 'Finalized rec count = ' || stat.final_rec || ' exceeds max allowed');
	end if;
	if interval > 1.1 * interval_max then --10% допуск
		raise_application_error(-20201, 'Archiver idle interval = ' || interval || ' exceeds max allowed');
	end if;
	--архиватор стартует не чаще, чем прописано в конфиге
	if interval > 1 then
		if stat.final_rec < final_max and interval < interval_max then
			raise_application_error(-20201, 'Archiver started earlier than expected: final rec = ' || stat.final_rec || ', interval = ' || interval);
		end if;
	end if;
	--то, что записи с arc = 'Y' выгребаются до конца не проверяю
end;
/

create or replace trigger smsc_msg_insert after insert on sms_msg
	referencing new as msg for each row
begin
	check_arc_stat;
	if :msg.svc_type = '-----' then
		raise_application_error(-20101, 'Abort sms_msg insert transaction test case: svc_type = ''-----''');
	end if;
	if :msg.st != 0 then
		raise_application_error(-20201, 'New message created with state != ENROTE');
	end if;
end;
/

create or replace trigger smsc_msg_update after update on sms_msg
	referencing new as msg for each row
begin
	if :msg.st != 0 then
		if :old.st != 0 then
			raise_application_error(-20201, 'Message prev state != ENROTE');
		end if;
		update arc_stat set final_rec = final_rec + 1;
	end if;
	check_arc_stat;
end;
/

create or replace trigger sms_msg_delete before delete on sms_msg
	referencing old as msg for each row
declare
	flag number;
begin
	--из sms_msg удаляются только те записи, что уже перенесены в sms_arc
	if :msg.arc = 'Y' then
		select count(*) into flag from sms_arc where id = :msg.id;
		if flag = 0 then
			raise_application_error(-20201, 'Trying to delete record in sms_msg with arc = ''Y'' that has no corresponding sms_arc record');
		end if;
	end if;
	--обновление arc_stat.last_arc делается в smsc_arc_insert
	if :msg.st = 0 then
		raise_application_error(-20201, 'Deleting message in ENROTE state');
	else
		update arc_stat set final_rec = final_rec - 1;
	end if;
end;
/

create or replace trigger smsc_arc_insert after insert on sms_arc
	referencing new as arc for each row
declare
	msg sms_msg%rowtype;
begin
	begin
		check_arc_stat;
		--проверка статуса
		if :arc.st = 0 then
			raise_application_error(-20201, 'arc.st = ENROTE');
		end if;
		--проверка признака архивации
		select * into msg from sms_msg where id = :arc.id;
		if msg.arc != 'Y' then
			raise_application_error(-20201, 'Trying to archivate sms with arc != ''Y''');
		end if;
		--обновление arc_stat.final_rec делается в sms_msg_delete
		update arc_stat set last_arc = sysdate;
		--проверка полей
		if msg.st != :arc.st then
			raise_application_error(-20201, 'msg.st != arc.st');
		end if;
		if msg.submit_time != :arc.submit_time then
			raise_application_error(-20201, 'msg.submit_time != arc.submit_time');
		end if;
		if msg.valid_time != :arc.valid_time then
			raise_application_error(-20201, 'msg.valid_time != arc.valid_time');
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
		--next_try_time skipped
		if msg.oa != :arc.oa then
			raise_application_error(-20201, 'msg.oa != arc.oa');
		end if;
		if msg.da != :arc.da then
			raise_application_error(-20201, 'msg.da != arc.da');
		end if;
		if msg.dda != :arc.dda then
			raise_application_error(-20201, 'msg.dda != arc.dda');
		end if;
		if msg.mr != :arc.mr then
			raise_application_error(-20201, 'msg.mr != arc.mr');
		end if;
		if msg.svc_type != :arc.svc_type then
			raise_application_error(-20201, 'msg.svc_type != arc.svc_type');
		end if;
		if msg.dr != :arc.dr then
			raise_application_error(-20201, 'msg.dr != arc.dr');
		end if;
		if msg.br != :arc.br then
			raise_application_error(-20201, 'msg.br != arc.br');
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
		if msg.dst_msc != :arc.dst_msc then
			raise_application_error(-20201, 'msg.dst_msc != arc.dst_msc');
		end if;
		if msg.dst_imsi != :arc.dst_imsi then
			raise_application_error(-20201, 'msg.dst_imsi != arc.dst_imsi');
		end if;
		if msg.dst_sme_n != :arc.dst_sme_n then
			raise_application_error(-20201, 'msg.dst_sme_n != arc.dst_sme_n');
		end if;
		if msg.route_id != :arc.route_id then
			raise_application_error(-20201, 'msg.route_id != arc.route_id');
		end if;
		if msg.svc_id != :arc.svc_id then
			raise_application_error(-20201, 'msg.svc_id != arc.svc_id');
		end if;
		if msg.prty != :arc.prty then
			raise_application_error(-20201, 'msg.prty != arc.prty');
		end if;
		if msg.src_sme_id != :arc.src_sme_id then
			raise_application_error(-20201, 'msg.src_sme_id != arc.src_sme_id');
		end if;
		if msg.dst_sme_id != :arc.dst_sme_id then
			raise_application_error(-20201, 'msg.dst_sme_id != arc.dst_sme_id');
		end if;
		if msg.txt_length != :arc.txt_length then
			raise_application_error(-20201, 'msg.txt_length != arc.txt_length');
		end if;
		if msg.body_len != :arc.body_len then
			raise_application_error(-20201, 'msg.body_len != arc.body_len');
		end if;
		if msg.body != :arc.body then
			raise_application_error(-20201, 'msg.body != arc.body');
		end if;
	exception
		when no_data_found then
			raise_application_error(-20201, 'No sms_msg record found for id = ' || :arc.id);
	end;
end;
/

