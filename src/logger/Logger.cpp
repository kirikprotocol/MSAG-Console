#include <stdio.h>

#include "Logger.h"

////////////////////////////// start of log library dependancy
#ifdef LOGGER_LIB_LOG4CPP
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/FileAppender.hh>
#else
#include <log4cplus/logger.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <logger/additional/SmscLayout.h>
#endif
////////////////////////////// end of log library dependancy

namespace smsc {
namespace logger {

bool Logger::isInitialized = false;

Logger Logger::getInstance(const std::string & name)
{
  if (!isInitialized)
  {
    Init("log4cpp.properties");
  }
  //snprintf();
  return Logger(name);
}

Logger Logger::getInstance(const char * const name)
{
  if (!isInitialized)
  {
    Init("log4cpp.properties");
  }
  //snprintf();
  return Logger(name);
}

Logger *_trace_cat;
Logger *_map_cat;
Logger *_mapdlg_cat;
Logger *_mapproxy_cat;
Logger *_sms_err_cat;

////////////////////////////// start of log library dependancy
#ifdef LOGGER_LIB_LOG4CPP
void Logger::Init(const std::string &configFileName)
{
  if(isInitialized)return;
  //fprintf(stderr,"Logger::Init: isInitialized=%s\n",isInitialized?"true":"false");
  try {
    isInitialized = false;
    log4cpp::Category::getRoot().removeAllAppenders();
	log4cpp::PropertyConfigurator::configure(configFileName);
  } catch (log4cpp::ConfigureFailure& exception) {
    fprintf( stderr, "Exception occured during configuration log4cpp: %s\n", exception.what() );
    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "smsc.log");
    appender->setLayout(new log4cpp::BasicLayout());
    log4cpp::Category & cat = log4cpp::Category::getRoot();
    cat.removeAllAppenders();
    cat.addAppender(appender);
    cat.setRootPriority(log4cpp::Priority::DEBUG);
  }
  isInitialized = true;
  _trace_cat=new Logger(getInstance("trace"));
  _map_cat=new Logger(getInstance("map"));
  _mapdlg_cat=new Logger(getInstance("map.dialog"));
  _mapproxy_cat=new Logger(getInstance("map.proxy"));
  _sms_err_cat=new Logger(getInstance("sms.error"));
}

void Logger::Shutdown()
{
  log4cpp::Category::shutdown();
  isInitialized = false;
}

Logger::LogLevel getLogLevelFromReal(const log4cpp::Priority::Value& level)
{
	switch (level)
	{
	case log4cpp::Priority::FATAL:
		return Logger::LEVEL_FATAL;
		break;
	case log4cpp::Priority::CRIT:
		return Logger::LEVEL_CRIT;
		break;
	case log4cpp::Priority::ERROR:
		return Logger::LEVEL_ERROR;
		break;
	case log4cpp::Priority::WARN:
		return Logger::LEVEL_WARN;
		break;
	case log4cpp::Priority::INFO:
		return Logger::LEVEL_INFO;
		break;
	case log4cpp::Priority::DEBUG:
		return Logger::LEVEL_DEBUG;
		break;
	case log4cpp::Priority::NOTSET:
		return Logger::LEVEL_NOTSET;
		break;
	default:
		return Logger::LEVEL_NOTSET;
		break;
	}
}

log4cpp::Priority::Value getRealLogLevel(const Logger::LogLevel &myLevel)
{
	switch (myLevel)
	{
	case Logger::LEVEL_FATAL:
		return log4cpp::Priority::FATAL;
		break;
	case Logger::LEVEL_CRIT:
		return log4cpp::Priority::CRIT;
		break;
	case Logger::LEVEL_ERROR:
		return log4cpp::Priority::ERROR;
		break;
	case Logger::LEVEL_WARN:
		return log4cpp::Priority::WARN;
		break;
	case Logger::LEVEL_INFO:
		return log4cpp::Priority::INFO;
		break;
	case Logger::LEVEL_DEBUG:
		return log4cpp::Priority::DEBUG;
		break;
	case Logger::LEVEL_NOTSET:
		return log4cpp::Priority::NOTSET;
		break;
	default:
		return log4cpp::Priority::NOTSET;
		break;
	}
}

Logger::LogCats* Logger::getCurrentCategories()
{
	typedef std::vector<log4cpp::Category*> Cats;
	Logger::LogCats * result = new Logger::LogCats();
	Cats* cats = log4cpp::Category::getCurrentCategories();
	for (Cats::iterator i = cats->begin(); i != cats->end(); i++)
	{
		log4cpp::Category * cat = *i;
		(*result)[cat->getName()] = getLogLevelFromReal(cat->getPriority());
	}
	delete cats;
	return result;
}

void Logger::setCategoryLogLevel(const char * const catName, Logger::LogLevel logLevel)
{
	log4cpp::Priority::Value priority = getRealLogLevel(logLevel);
	if (strlen(catName) != 0)
	{
		log4cpp::Category &cat = log4cpp::Category::getInstance(catName);
		if (cat.getPriority() != priority)
		{
			cat.setPriority(priority);
		}
	}
	else
	{
		if (priority != log4cpp::Priority::NOTSET)
		{
			log4cpp::Category & cat = log4cpp::Category::getRoot();
			if (cat.getPriority() != priority)
			{
				log4cpp::Category::setRootPriority(priority);
			}
		}
	}
}

Logger::Logger(const char * const logCategoryName)
	:logger(log4cpp::Category::getInstance(logCategoryName))
{
}

Logger::Logger(const std::string & logCategoryName)
	:logger(log4cpp::Category::getInstance(logCategoryName))
{
}

Logger::Logger(const Logger& copy)
	:logger(copy.logger)
{
}



bool Logger::isLogLevelEnabled(const LogLevel logLevel) const throw()
{
	return logger.isPriorityEnabled(getRealLogLevel(logLevel));
}

void Logger::log(const LogLevel logLevel, const std::string &message) throw()
{
	logger.log(getRealLogLevel(logLevel), message);
}

void Logger::logva(const LogLevel logLevel, const char * const stringFormat, va_list args) throw()
{
	logger.logva(getRealLogLevel(logLevel), stringFormat, args);
}

///////////////////////////////////////////////////////////////////
#else //#ifdef LOGGER_LIB_LOG4CPP
///////////////////////////////////////////////////////////////////

void Logger::Init(const std::string &configFileName)
{
  if(isInitialized)return;
  //fprintf(stderr,"Logger::Init: isInitialized=%s\n",isInitialized?"true":"false");
  try {
    isInitialized = false;
	log4cplus::Logger::getDefaultHierarchy().resetConfiguration();
	std::auto_ptr<log4cplus::spi::LayoutFactory> smscLayoutFactory(new smsc::logger::SmscLayoutFactory());
	log4cplus::spi::getLayoutFactoryRegistry().put(smscLayoutFactory);
    log4cplus::PropertyConfigurator::doConfigure(configFileName);
	if( log4cplus::Logger::getRoot().getAllAppenders().size() != 0 ) {
		isInitialized = true;
	}
  } catch (...) {
	  //fprintf( stderr, "Exception occured during configuration log4cpp: %s\n", exception.what() );
	  fprintf( stderr, "Unknown exception occured during configuration log4cpp\n");
  }
  if( !isInitialized ) {
    try {
	  log4cplus::SharedAppenderPtr append_1(new log4cplus::RollingFileAppender("smsc.log", 5*1024, 5));
	  append_1->setName("Default");
	  append_1->setLayout( std::auto_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout()) );
	  log4cplus::Logger::getRoot().addAppender(append_1);
	  log4cplus::Logger::getRoot().setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
    } catch (...) {
	  fprintf( stderr, "Can't configure logger by default. No logging will be available\n");
    }
  }
  isInitialized = true;
  _trace_cat=new Logger(getInstance("trace"));
  _map_cat=new Logger(getInstance("map"));
  _mapdlg_cat=new Logger(getInstance("map.dialog"));
  _mapproxy_cat=new Logger(getInstance("map.proxy"));
  _sms_err_cat=new Logger(getInstance("sms.error"));
}

void Logger::Shutdown()
{
	log4cplus::Logger::getDefaultHierarchy().shutdown();
	isInitialized = false;
}

Logger::LogLevel getLogLevelFromReal(const log4cplus::LogLevel& level)
{
	switch (level)
	{
	case log4cplus::FATAL_LOG_LEVEL:
		return Logger::LEVEL_FATAL;
		break;
/*	case log4cplus::CRIT:
		return Logger::LEVEL_CRIT;
		break;*/
	case log4cplus::ERROR_LOG_LEVEL:
		return Logger::LEVEL_ERROR;
		break;
	case log4cplus::WARN_LOG_LEVEL:
		return Logger::LEVEL_WARN;
		break;
	case log4cplus::INFO_LOG_LEVEL:
		return Logger::LEVEL_INFO;
		break;
	case log4cplus::DEBUG_LOG_LEVEL:
		return Logger::LEVEL_DEBUG;
		break;
	case log4cplus::NOT_SET_LOG_LEVEL:
		return Logger::LEVEL_NOTSET;
		break;
	default:
		return Logger::LEVEL_NOTSET;
		break;
	}
}

log4cplus::LogLevel getRealLogLevel(const Logger::LogLevel &myLevel)
{
	switch (myLevel)
	{
	case Logger::LEVEL_FATAL:
		return log4cplus::FATAL_LOG_LEVEL;
		break;
	case Logger::LEVEL_ERROR:
		return log4cplus::ERROR_LOG_LEVEL;
		break;
	case Logger::LEVEL_WARN:
		return log4cplus::WARN_LOG_LEVEL;
		break;
	case Logger::LEVEL_INFO:
		return log4cplus::INFO_LOG_LEVEL;
		break;
	case Logger::LEVEL_DEBUG:
		return log4cplus::DEBUG_LOG_LEVEL;
		break;
	case Logger::LEVEL_NOTSET:
		return log4cplus::NOT_SET_LOG_LEVEL;
		break;
	default:
		return log4cplus::NOT_SET_LOG_LEVEL;
		break;
	}
}

Logger::LogCats* Logger::getCurrentCategories()
{
	Logger::LogCats * result = new Logger::LogCats();
	(*result)[""] = getLogLevelFromReal(log4cplus::Logger::getRoot().getLogLevel());
	log4cplus::LoggerList list = log4cplus::Logger::getCurrentLoggers();
	for (log4cplus::LoggerList::iterator i = list.begin(); i != list.end(); i++)
	{
		//log4cplus::Logger l = *i;
		(*result)[i->getName()] = getLogLevelFromReal(i->getLogLevel());
	}
	return result;
}

void Logger::setCategoryLogLevel(const char * const catName, Logger::LogLevel logLevel)
{
	log4cplus::LogLevel priority = getRealLogLevel(logLevel);
	if (strlen(catName) != 0)
	{
		log4cplus::Logger cat = log4cplus::Logger::getInstance(catName);

		if (cat.getLogLevel() != priority)
		{
			cat.setLogLevel(priority);
		}
	}
	else
	{
		if (priority != log4cplus::NOT_SET_LOG_LEVEL)
		{
			log4cplus::Logger cat = log4cplus::Logger::getRoot();
			if (cat.getLogLevel() != priority)
			{
				cat.setLogLevel(priority);
			}
		}
	}
}

Logger::Logger(const char * const logCategoryName)
	:logger(log4cplus::Logger::getInstance(logCategoryName))
{
}
Logger::Logger(const std::string & logCategoryName)
	:logger(log4cplus::Logger::getInstance(logCategoryName))
{
}

Logger::Logger(const Logger& copy)
	:logger(copy.logger)
{
}


bool Logger::isLogLevelEnabled(const LogLevel logLevel) const throw()
{
	return logger.isEnabledFor(getRealLogLevel(logLevel));
}

void Logger::log(const LogLevel logLevel, const std::string &message) throw()
{
	logger.log(getRealLogLevel(logLevel), message);
}

std::string toStringForm(const char * const format, va_list args)
{
	size_t size = 1024;
	char* buffer = new char[size];
	while (1) {
	    int n = vsnprintf(buffer, size, format, args);
		
		// If that worked, return a string.
		if ((n > -1) && (static_cast<size_t>(n) < size))
		{
			std::string s(buffer);
			delete [] buffer;
			return s;
		}
		
		// Else try again with more space.
		size = (n > -1) 
			? n + 1     // ISO/IEC 9899:1999
			: size * 2; // twice the old size
		
		delete [] buffer;
		buffer = new char[size];
	}
}

void Logger::logva(const LogLevel logLevel, const char * const stringFormat, va_list args) throw()
{
	std::string str = toStringForm(stringFormat, args);
	log(logLevel, str);
}

#endif //#ifdef LOGGER_LIB_LOG4CPP
////////////////////////////// end of log library dependancy

const char * const Logger::getLogLevelName(const LogLevel level) throw()
{
	switch (level)
	{
	case Logger::LEVEL_FATAL:
		return "FATAL";
		break;
	case Logger::LEVEL_ERROR:
		return "ERROR";
		break;
	case Logger::LEVEL_WARN:
		return "WARN";
		break;
	case Logger::LEVEL_INFO:
		return "INFO";
		break;
	case Logger::LEVEL_DEBUG:
		return "DEBUG";
		break;
	case Logger::LEVEL_NOTSET:
		return "NOTSET";
		break;
	default:
		return "NOTSET";
		break;
	}
}

const Logger::LogLevel Logger::getLogLevel(const char * const logLevelName) throw()
{
	if (strcmp("FATAL", logLevelName) == 0)
		return Logger::LEVEL_FATAL;
	if (strcmp("ERROR", logLevelName) == 0)
		return Logger::LEVEL_ERROR;
	if (strcmp("WARN", logLevelName) == 0)
		return Logger::LEVEL_WARN;
	if (strcmp("INFO", logLevelName) == 0)
		return Logger::LEVEL_INFO;
	if (strcmp("DEBUG", logLevelName) == 0)
		return Logger::LEVEL_DEBUG;
	return Logger::LEVEL_NOTSET;
}

void Logger::log(const LogLevel logLevel, const char * const stringFormat, ...) throw()
{
	va_list args;
	va_start(args, stringFormat);
	logva(logLevel, stringFormat, args);
	va_end(args);
}


void Logger::fatal(const char* stringFormat, ...) throw()
{
	va_list args;
	va_start(args, stringFormat);
	logva(Logger::LEVEL_FATAL, stringFormat, args);
	va_end(args);
}

void Logger::fatal(const std::string& message) throw()
{
	log(Logger::LEVEL_FATAL, message);
}

void Logger::error(const char* stringFormat, ...) throw()
{
	va_list args;
	va_start(args, stringFormat);
	logva(Logger::LEVEL_ERROR, stringFormat, args);
	va_end(args);
}

void Logger::error(const std::string& message) throw()
{
	log(Logger::LEVEL_ERROR, message);
}

void Logger::warn(const char* stringFormat, ...) throw()
{
	va_list args;
	va_start(args, stringFormat);
	logva(Logger::LEVEL_WARN, stringFormat, args);
	va_end(args);
}

void Logger::warn(const std::string& message) throw()
{
	log(Logger::LEVEL_WARN, message);
}

void Logger::info(const char* stringFormat, ...) throw()
{
	va_list args;
	va_start(args, stringFormat);
	logva(Logger::LEVEL_INFO, stringFormat, args);
	va_end(args);

}

void Logger::info(const std::string& message) throw()
{
	log(Logger::LEVEL_INFO, message);
}

void Logger::debug(const char* stringFormat, ...) throw()
{
	va_list args;
	va_start(args, stringFormat);
	logva(Logger::LEVEL_DEBUG, stringFormat, args);
	va_end(args);
}

void Logger::debug(const std::string& message) throw()
{
	log(Logger::LEVEL_DEBUG, message);
}

}
}
