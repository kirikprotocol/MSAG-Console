drop table db_sme_test;
create table db_sme_test
(  
	id number(10),
	int8 number(3),
	int16 number(5),
	int32 number(10),
	int64 number(20),
	flt number(5,2),
	dbl number(5,2),
	ldbl number(5,2),
	str varchar2(200),
	dt date
) tablespace smsc_data;

create unique index db_sme_test_idx on db_sme_test (id)
tablespace smsc_idx;

alter table db_sme_test add constraint db_sme_test_pk primary key (id);

create or replace procedure test_proc (int8 in out smallint, int16 in out smallint,
	int32 in out integer, int64 in out integer, flt in out float,
	dbl in out double precision, ldbl in out double precision,
	str in out varchar2, dt in out date) is
begin
	int8 := int8 + 1;
	int16 := int16 + 1;
	int32 := int32 + 1;
	int64 := int64 + 1;
	flt := flt + 1.1;
	dbl := dbl + 1.1;
	ldbl := ldbl + 1.1;
	str := str || '!!!';
	dt := dt + 3600;
end;

create or replace function test_func (int8 in out smallint, int16 in out smallint,
	int32 in out integer, int64 in out integer, flt in out float,
	dbl in out double precision, ldbl in out double precision,
	str in out varchar2, dt in out date) return number is
begin
	int8 := int8 + 1;
	int16 := int16 + 1;
	int32 := int32 + 1;
	int64 := int64 + 1;
	flt := flt + 1.1;
	dbl := dbl + 1.1;
	ldbl := ldbl + 1.1;
	str := str || '!!!';
	dt := dt + 3600;
	return 5;
end;

create or replace procedure test_exception(param in number) is
begin
	if param = 1 then --system exception
		raise VALUE_ERROR;
	elsif param = 2 then --user error
		raise_application_error(-22222, 'Test user-defined exception');
	else
		raise INVALID_NUMBER;
	end if;
end;
