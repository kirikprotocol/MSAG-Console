create or replace procedure insert_record(msg sms_msg%rowtype) is
begin
	insert into sms_msg (id,st,submit_time,valid_time,attempts,last_result,
		last_try_time,next_try_time,oa,da,dda,mr,svc_type,arc,dr,br,src_msc,
		src_imsi,src_sme_n,dst_msc,dst_imsi,dst_sme_n,body_len,body)
	values (msg.id,msg.st,msg.submit_time,msg.valid_time,msg.attempts,
		msg.last_result,msg.last_try_time,msg.next_try_time,msg.oa,msg.da,
		msg.dda,msg.mr,msg.svc_type,msg.arc,msg.dr,msg.br,msg.src_msc,
		msg.src_imsi,msg.src_sme_n,msg.dst_msc,msg.dst_imsi,msg.dst_sme_n,
		msg.body_len,msg.body);
end;
/

create or replace procedure change_status(id_ int, st_ number) is
begin
	update sms_msg set st = st_ where id = hextoraw(id_);
end;
/

create or replace procedure test(num number) is
	last_id number;
	max_id number;
	i number;
	t1 date;
	t2 date;
	dt float;
	n1 number;
	n2 number;
	dn number;
	rate float;
	msg sms_msg%rowtype;
begin
	select rawtohex(nvl(max(id), hextoraw('0'))) into last_id from sms_msg;
	i := last_id + 1;
	max_id := last_id + num;
	select count(*) into n1 from sms_msg; 
	select sysdate into t1 from dual;
	while i <= max_id loop
		--createSms()
		msg.id := to_char(i, 'fm0999999999999999');
		msg.st := 0;
		msg.submit_time := sysdate;
		msg.valid_time := sysdate;
		msg.attempts := 5;
		msg.last_result := 105;
		msg.last_try_time := sysdate;
		msg.next_try_time := sysdate;
		msg.oa := '101.101.12345678901234567890';
		msg.da := '102.102.12345678901234567890';
		msg.dda := '103.103.12345678901234567890';
		msg.mr := 69;
		msg.svc_type := '123456';
		msg.arc := 'Y';
		msg.dr := 106;
		msg.br := 101;
		msg.src_msc := '123456789012345678901';
		msg.src_imsi := '123456789012345678901';
		msg.src_sme_n := 123456789;
		msg.dst_msc := '123456789012345678901';
		msg.dst_imsi := '123456789012345678901';
		msg.dst_sme_n := 123456789;
		msg.body_len := 550; -- = 18*12 (int fileds) + 254 + 8 (short_message) + 64 + 8 (receipted_message_id)
		msg.body := hextoraw('313839453435338414c4947495946475559464359544448474b4d54f4a494849464448544b485495554f5949465554464c494a4f49555494f594f4947484b55484b4c554748594a46555446545548544459525345524145575847564e42494a54f4a4f549484959475954445246434e424859465954445254525344545344554755544446595244544452434b42484b4c4a494f4849475546544452534454455346484c4a4f4954a554f4946544454444a48564b48554f5949484c49484759464a54464a4c4c4c4313839453435338414c4947495946475559464359544448474b4d54f4a494849464448544b485495554f5949465554464c494a4f49555494f594f4947484b55484b4c554748594a46555446545548544459525345524145575847564e42494a54f4a4f549484959475954445246434e424859465954445254525344545344554755544446595244544452434b42484b4c4a494f4849475546544452534454455346484c4a4f4954a554f4946544454444a48564b48554f5949484c49484759464a54464a4c4c4c4313839453435338414c4947495946475559464359544448474b4d54f4a494849464448544b485495554f5949465554464c494a4f49555494f594f4947484b55484b4c554748594a46555446545548544459525345524145575847564e42494a54f4a4f549484959475954445246434e424859465954445254525344545344554755544446595244544452434b42484b4c4a494f4849471');
		--msg.body_len := 200;
		--msg.body := hextoraw('313839453435338414c4947495946475559464359544448474b4d54f4a494849464448544b485495554f5949465554464c494a4f49555494f594f4947484b55484b4c554748594a46555446545548544459525345524145575847564e42494a54f4a4f549484959475954445246434e424859465954445254525344545344554755544446595244544452434b42484b4c4a494f4849475546544452534454455346484c4a4f4954a554f4946544454444a48564b48554f5949484c49484759464a54464a4c4c4c4');

		insert_record(msg);
		commit;
		--changeSmsState()
        change_status(i, 1);
		commit;
		i := i + 1;
	end loop;
	select sysdate into t2 from dual;
	select count(*) into n2 from sms_msg; 
	dt := (t2 - t1) * 3600 * 24;
	dn := n2 - n1;
	if dt > 0 then
		rate := num / dt;
	else
		rate := 0;
	end if;
	dbms_output.put_line('dt = ' || dt || ', dn = ' || dn || ', rate = ' || rate);
end;
/

--execute test(100);
