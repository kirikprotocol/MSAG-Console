	$Id$

	Logging:
	подключаете "logger/Logger.h"
	метод smsc::logger::Logger.getInstance(const std::string & categoryName) 
	возвращает smsc::logger::Logger, с помощью которго и производится запись 
	в лог. Имя категории состоит из полного имени класса, где "::" 
	заменяются на ".". Например, для класса "smsc::util::SMyUtilClass" имя 
	категории будет "smsc.util.MyUtilClass".
	Подробнее читайте docs
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
	Важно: Для получения Logger
		используйте smsc::logger::Logger.getInstance();
		
