--Все коннекты к БД одинаково равномерно загружены
select stat.sid, stat.statistic#, nam.name, stat.value from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# in (4,5,6) and
ses.machine='smsc' and ses.osuser='bryz'
order by statistic#, value desc


--Восстановление конектов к БД после alter system kill session
select sid, serial#, status from v$session where machine='smsc' and osuser='bryz';
select 'alter system kill session ''' || sid || ',' || serial# || ''';' from v$session where machine='smsc' and osuser='bryz' and status != 'KILLED'
alter system kill session '13,8118';


--Работа с БД с большим размером пула соединений
select count(*) from v$session where machine='smsc' and osuser='bryz';


select stat.statistic#, nam.name, stat.value from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# = 4 and
ses.machine='smsc' and ses.osuser='bryz'
order by stat.value


--Поведение Message Store при временной блокировке всей таблицы
lock table sms_msg in exclusive mode
commit


