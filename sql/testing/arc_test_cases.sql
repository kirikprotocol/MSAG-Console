--Процесс работы архиватора
select (select count(*) from sms_msg where st != 0) msg2arc, (select count(*) from sms_arc) arc from dual

--Все сообщения с финальным статусом переносятся из sms_msg в архив
select st, count(*) from sms_msg group by st order by st

--В архиве сообщения только с финальном статусом
select st, count(*) from sms_arc group by st order by st

--Записи в sms_msg и sms_arc не пересекаются
select count(*) from sms_msg msg, sms_arc arc where msg.id = arc.id

