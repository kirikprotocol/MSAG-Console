	$Id$

	Logging:
	����������� "util/Logger.h"
	����� smsc::util::Logger.getCategory(const std::string & categoryName) 
	���������� log4cpp::Category, � ������� ������� � ������������ ������ 
	� ���. ��� ��������� ������� �� ������� ����� ������, ��� "::" 
	���������� �� ".". ��������, ��� ������ "smsc::util::SMyUtilClass" ��� 
	��������� ����� "smsc.util.MyUtilClass".
	��������� ������� docs/log4cpp-docs-0.2.7/classlog4cpp_1_1Category.html
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
	�����: ��� ��������� log4cpp::Category 
		����������� smsc::util::Logger.getCategory(), 
		� �� log4cpp::Category.getInstance().
		
