--������� ������ ����������
select (select count(*) from sms_msg where st != 0) msg2arc, (select count(*) from sms_arc) arc from dual

--��� ��������� � ��������� �������� ����������� �� sms_msg � �����
select st, count(*) from sms_msg group by st order by st

--� ������ ��������� ������ � ��������� ��������
select st, count(*) from sms_arc group by st order by st

--������ � sms_msg � sms_arc �� ������������
select count(*) from sms_msg msg, sms_arc arc where msg.id = arc.id

