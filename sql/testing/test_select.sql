procedure select_record(src varchar2, dest varchar2, servtype varchar2) is
	smsid sms_msg.id%type;
begin
	select id into smsid from sms_msg where oa = src and da = dest and svc_type = servtype;
exception
	;
end;
/

create or replace procedure test_select(num number) is
	i number;
	t1 date;
	t2 date;
	dt float;
	rate float;
begin
	i := 0;
	select sysdate into t1 from dual;
	while i <= num loop
		select_record('.238.200.f13fMaIB9EL5fq1gGY', '.2.1.821999503282047102', 'jtunY');
		i := i + 1;
	end loop;
	select sysdate into t2 from dual;
	dt := (t2 - t1) * 3600 * 24;
	if dt > 0 then
		rate := num / dt;
	else
		rate := 0;
	end if;
	dbms_output.put_line('dt = ' || dt || ', num = ' || num || ', rate = ' || rate);

end;
/
--execute test(100);
