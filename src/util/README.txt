	$Id$

	Logging:
	����������� "logger/Logger.h"
	����� smsc::logger::Logger.getInstance(const std::string & categoryName) 
	���������� smsc::logger::Logger, � ������� ������� � ������������ ������ 
	� ���. ��� ��������� ������� �� ������� ����� ������, ��� "::" 
	���������� �� ".". ��������, ��� ������ "smsc::util::SMyUtilClass" ��� 
	��������� ����� "smsc.util.MyUtilClass".
	��������� ������� docs
	������:
		emerg (const std::string &message)
		fatal (const char *stringFormat,...)
		alert (const char *stringFormat,...)
		crit (const char *stringFormat,...)
		error (const char *stringFormat,...)
		warn (const char *stringFormat,...)
		notice (const char *stringFormat,...)
		info (const char *stringFormat,...)
		debug (const char *stringFormat,...)
	�����: ��� ��������� Logger
		����������� smsc::logger::Logger.getInstance();
		
