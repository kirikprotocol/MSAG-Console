/*
	Тест кейсы:
	 - В архиве сообщения только с финальным статусом
	 - Записи не теряются при переносе из sms_msg в sms_arc
	 - Данные не искажаются при переносе из sms_msg в sms_arc
	 - Все сообщения с финальным статусом переносятся из sms_msg в архив
	 - Только сообщения с установленным признаком архивации переносятся в архив
*/

create or replace trigger smsc_msg_insert after insert on sms_msg
	referencing new as msg for each row
begin
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
		raise_application_error(-20201, 'Message state != ENROTE');
	end if;
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
	if :msg.st != 0 then
		raise_application_error(-20201, 'Message must be in ENROTE state');
	end if;
end;
/

create or replace function chk_num(val1 in number, val2 in number) return boolean is
begin
    if (val1 is null) and (val2 is null) then
 	   return true;
    elsif (val1 is not null) and (val2 is not null) and (val1 = val2) then
 	   return true;
    else
 	   return false;
    end if;
end;
/

create or replace function chk_date(val1 in date, val2 in date) return boolean is
begin
    if (val1 is null) and (val2 is null) then
 	   return true;
    elsif (val1 is not null) and (val2 is not null) and (val1 = val2) then
 	   return true;
    else
 	   return false;
    end if;
end;
/

create or replace function chk_str(val1 in varchar2, val2 in varchar2) return boolean is
begin
    if (val1 is null) and (val2 is null) then
 	   return true;
    elsif (val1 is not null) and (val2 is not null) and (val1 = val2) then
 	   return true;
    else
 	   return false;
    end if;
end;
/

create or replace function chk_raw(val1 in raw, val2 in raw) return boolean is
begin
    if (val1 is null) and (val2 is null) then
 	   return true;
    elsif (val1 is not null) and (val2 is not null) and (val1 = val2) then
 	   return true;
    else
 	   return false;
    end if;
end;
/

create or replace trigger smsc_arc_insert after insert on sms_arc
	referencing new as arc for each row
declare
	msg sms_msg%rowtype;
begin
	begin
		--проверка статуса
		if :arc.st = 0 then
			raise_application_error(-20201, 'arc.st = ENROTE');
		end if;
		--проверка признака архивации
		select * into msg from sms_msg where id = :arc.id;
		if msg.arc != 'Y' then
			raise_application_error(-20201, 'Trying to archivate sms with arc != ''Y''');
		end if;
		--проверка полей
		--st skipped
		if not chk_date(msg.submit_time, :arc.submit_time) then
			raise_application_error(-20201, 'msg.submit_time != arc.submit_time');
		end if;
		if not chk_date(msg.valid_time, :arc.valid_time) then
			raise_application_error(-20201, 'msg.valid_time != arc.valid_time');
		end if;
		--attempts skipped
		--last_result skipped
		--last_try_time skipped
		--next_try_time skipped
		if not chk_str(msg.oa, :arc.oa) then
			raise_application_error(-20201, 'msg.oa != arc.oa');
		end if;
		if not chk_str(msg.da, :arc.da) then
			raise_application_error(-20201, 'msg.da != arc.da');
		end if;
		if not chk_str(msg.dda, :arc.dda) then
			raise_application_error(-20201, 'msg.dda != arc.dda');
		end if;
		if not chk_num(msg.mr, :arc.mr) then
			raise_application_error(-20201, 'msg.mr != arc.mr');
		end if;
		if not chk_str(msg.svc_type, :arc.svc_type) then
			raise_application_error(-20201, 'msg.svc_type != arc.svc_type');
		end if;
		if not chk_num(msg.dr, :arc.dr) then
			raise_application_error(-20201, 'msg.dr != arc.dr');
		end if;
		if not chk_num(msg.br, :arc.br) then
			raise_application_error(-20201, 'msg.br != arc.br');
		end if;
		if not chk_str(msg.src_msc, :arc.src_msc) then
			raise_application_error(-20201, 'msg.src_msc != arc.src_msc');
		end if;
		if not chk_str(msg.src_imsi, :arc.src_imsi) then
			raise_application_error(-20201, 'msg.src_imsi != arc.src_imsi');
		end if;
		if not chk_num(msg.src_sme_n, :arc.src_sme_n) then
			raise_application_error(-20201, 'msg.src_sme_n != arc.src_sme_n');
		end if;
		--dst_msc skipped
		--dst_imsi skipped
		--dst_sme_n skipped
		if not chk_str(msg.route_id, :arc.route_id) then
			raise_application_error(-20201, 'msg.route_id != arc.route_id');
		end if;
		if not chk_num(msg.svc_id, :arc.svc_id) then
			raise_application_error(-20201, 'msg.svc_id != arc.svc_id');
		end if;
		if not chk_num(msg.prty, :arc.prty) then
			raise_application_error(-20201, 'msg.prty != arc.prty');
		end if;
		if not chk_str(msg.src_sme_id, :arc.src_sme_id) then
			raise_application_error(-20201, 'msg.src_sme_id != arc.src_sme_id');
		end if;
		if not chk_str(msg.dst_sme_id, :arc.dst_sme_id) then
			raise_application_error(-20201, 'msg.dst_sme_id != arc.dst_sme_id');
		end if;
		if not chk_num(msg.txt_length, :arc.txt_length) then
			raise_application_error(-20201, 'msg.txt_length != arc.txt_length');
		end if;
		if not chk_num(msg.body_len, :arc.body_len) then
			raise_application_error(-20201, 'msg.body_len != arc.body_len');
		end if;
		if not chk_raw(msg.body, :arc.body) then
			raise_application_error(-20201, 'msg.body != arc.body');
		end if;
	exception
		when no_data_found then
			raise_application_error(-20201, 'No sms_msg record found for id = ' || :arc.id);
	end;
end;
/

