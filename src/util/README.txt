	$Id$

	Logging:
	подключаете "logger/Logger.h"
	метод smsc::util::Logger.getCategory(const std::string & categoryName) 
	возвращает log4cpp::Category, с помощью которго и производится запись 
	в лог. Имя категории состоит из полного имени класса, где "::" 
	заменяются на ".". Например, для класса "smsc::util::SMyUtilClass" имя 
	категории будет "smsc.util.MyUtilClass".
	Подробнее читайте docs/log4cpp-docs-0.2.7/classlog4cpp_1_1Category.html
	методы:
		emerg (const std::string &message)
		fatal (const char *stringFormat,...)
		alert (const char *stringFormat,...)
		crit (const char *stringFormat,...)
		error (const char *stringFormat,...)
		warn (const char *stringFormat,...)
		notice (const char *stringFormat,...)
		info (const char *stringFormat,...)
		debug (const char *stringFormat,...)
	Важно: Для получения log4cpp::Category 
		используйте smsc::util::Logger.getCategory(), 
		а не log4cpp::Category.getInstance().
		
