/*
	���� �����:
	 - � ������ ��������� ������ � ��������� ��������
	 - ������ �� �������� ��� �������� �� sms_msg � sms_arc
	 - ������ �� ���������� ��� �������� �� sms_msg � sms_arc
	 - ��� ��������� � ��������� �������� ����������� �� sms_msg � �����
	 - ������ ��������� � ������������� ��������� ��������� ����������� � �����
*/

drop table arc_stat;
create table arc_stat
(
	final_rec integer,
	last_arc date
) tablespace smsc_data;
insert into arc_stat (final_rec, last_arc) values (0, sysdate + 1/24);

--�������� ������� �� ��������� ����������
create or replace procedure check_arc_stat is
	final_max integer := 100; --config.xml: MessageStore/Archive/finalized
    interval_max integer := 20; --config.xml: MessageStore/Archive/interval
	final_check_accuracy integer := 5;
	interval_check_accuracy integer := 1;
	stat arc_stat%rowtype;
	interval integer;
begin
	select * into stat from arc_stat;
	interval := sysdate - stat.last_arc;
	--��������� �������� �� ����, ��� ��������� � �������
	if stat.final_rec > (final_max + final_check_accuracy) then
		raise_application_error(-20201, 'Finalized rec count = ' || stat.final_rec || ' exceeds max allowed');
	end if;
	if interval > (interval_max + interval_check_accuracy) then
		raise_application_error(-20201, 'Archiver idle interval = ' || interval || ' exceeds max allowed');
	end if;
	--��������� �������� �� ����, ��� ��������� � �������
	if interval > 1 then
		if stat.final_rec < final_max and interval < interval_max then
			raise_application_error(-20201, 'Archiver started earlier than expected: final rec = ' || stat.final_rec || ', interval = ' || interval);
		end if;
	end if;
	--��, ��� ������ � arc = 'Y' ����������� �� ����� �� ��������
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
	--�� sms_msg ��������� ������ �� ������, ��� ��� ���������� � sms_arc
	if :msg.arc = 'Y' then
		select count(*) into flag from sms_arc where id = :msg.id;
		if flag = 0 then
			raise_application_error(-20101, 'Trying to delete record in sms_msg with arc = ''Y'' that has no corresponding sms_arc record');
		end if;
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
		--state
		if :arc.st = 0 then
			raise_application_error(-20201, 'arc.st = ENROTE');
		end if;
		--arc = 'Y'
		select * into msg from sms_msg where id = :arc.id;
		if msg.arc != 'Y' then
			raise_application_error(-20201, 'Trying to archivate sms with arc != ''Y''');
		end if;
		--update arc_stat
		update arc_stat set final_rec =  final_rec - 1, last_arc = sysdate;
		--data
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

