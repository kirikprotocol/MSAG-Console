drop table db_sme_test;
create table db_sme_test
(  
	id number(10),
	int16 number(5),
	int32 number(10),
	flt number(3,2),
	dbl number(3,2),
	dt date,
	str varchar2(200)
) tablespace smsc_data;

create unique index db_sme_test_idx on db_sme_test (id)
tablespace smsc_idx;

alter table db_sme_test add constraint db_sme_test_pk primary key (id);
