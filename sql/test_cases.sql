--�������������� �������� � �� ����� alter system kill session
select sid, serial#, status from v$session where machine='smsc' and osuser='bryz';
select 'alter system kill session ''' || sid || ',' || serial# || ''';' from v$session where machine='smsc' and osuser='bryz'
alter system kill session '13,8118';

--������ � �� � ������� �������� ���� ����������
select count(*) from v$session where machine='smsc' and osuser='bryz';

--��� �������� � �� ��������� ���������� ���������
select stat.statistic#, nam.name, max(stat.value), min(stat.value) from v$sesstat stat, v$session ses, v$statname nam
where stat.sid = ses.sid and stat.statistic# = nam.statistic# and stat.statistic# in (4,5,6) and
ses.machine='smsc' and ses.osuser='bryz'
group by stat.statistic#, nam.name

--��������� Message Store ��� ��������� ���������� ���� �������
lock table sms_msg in exclusive mode
commit

