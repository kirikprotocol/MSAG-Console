create or replace procedure insert_record(id_ int, st_ number, mr_ number, rm_ number, oa_len_ number, oa_ton_ number, oa_npi_ number, oa_val_ varchar2, da_len_ number, da_ton_ number, da_npi_ number, da_val_ varchar2, valid_time_ date, wait_time_ date, submit_time_ date, delivery_time_ date, srr_ char, rd_ char, pri_ number, pid_ number, fcs_ number, dcs_ number, udhi_ char, udl_ number, ud_ raw) is
begin
	insert into sms_msg (id, st, mr, rm, oa_len, oa_ton, oa_npi, oa_val, da_len, da_ton, da_npi, da_val, valid_time, wait_time,	submit_time, delivery_time, srr, rd, pri, pid, fcs, dcs, udhi, udl, ud)
	values (id_, st_, mr_, rm_, oa_len_, oa_ton_, oa_npi_, oa_val_, da_len_, da_ton_, da_npi_, da_val_, valid_time_, wait_time_, submit_time_, delivery_time_, srr_, rd_, pri_, pid_, fcs_, dcs_, udhi_, udl_, ud_);
end;
/

create or replace procedure change_status(id_ int, st_ number) is
begin
	update sms_msg set st = st_ where id = id_;
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
begin
	select nvl(max(id),0) into last_id from sms_msg;
	i := last_id + 1;
	max_id := last_id + num;
	select count(*) into n1 from sms_msg; 
	select sysdate into t1 from dual;
	while i <= max_id loop
		--sms_msg:    id, st, mr, rm, oa_len, oa_ton, oa_npi,                  oa_val, da_len, da_ton, da_npi,                  da_val, valid_time, wait_time, submit_time, delivery_time, srr,  rd, pri, pid, fcs, dcs, udhi, udl, ud
		insert_record( i,  0, 69, 71,     21,     20,     30, '123456789012345678901',     21,     20,     30, '123456789012345678901',    sysdate,   sysdate,     sysdate,       sysdate, 'y', 'y', 100, 150,  55,  20,  'n', 200, hextoraw('313839453435338414c4947495946475559464359544448474b4d54f4a494849464448544b485495554f5949465554464c494a4f49555494f594f4947484b55484b4c554748594a46555446545548544459525345524145575847564e42494a54f4a4f549484959475954445246434e424859465954445254525344545344554755544446595244544452434b42484b4c4a494f4849475546544452534454455346484c4a4f4954a554f4946544454444a48564b48554f5949484c49484759464a54464a4c4c4c4'));
		commit;
        change_status(i, 1);
		commit;
		i := i + 1;
	end loop;
    /*
	i := last_id + 1;
	while i <= max_id loop
        change_status(i, 1);
		commit;
		i := i + 1;
	end loop;
    */
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
