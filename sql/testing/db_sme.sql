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
