#ifndef SMSC_LOGGER_H_INCLUDED_C3A87A6B
#define SMSC_LOGGER_H_INCLUDED_C3A87A6B

#include <stdarg.h>
#include <core/buffers/Hash.hpp>
#include <core/synchronization/Mutex.hpp>
#include <util/Exception.hpp>
#include <util/Properties.h>

#include "logger/Appender.h"
#include "logger/additional/ConfigReader.h"


#define smsc_log_fatal(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isFatalEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_FATAL, __VA_ARGS__)
#define smsc_log_error(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isErrorEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_ERROR, __VA_ARGS__)
#define smsc_log_warn(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isWarnEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_WARN, __VA_ARGS__)
#define smsc_log_info(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isInfoEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_INFO, __VA_ARGS__)
#define smsc_log_debug(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isDebugEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_DEBUG, __VA_ARGS__)


using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace smsc::util;

namespace smsc {
namespace logger {

class Logger
{
public:
	typedef enum {
		LEVEL_FATAL = 5,
		LEVEL_ERROR = 4,
		LEVEL_WARN = 3,
		LEVEL_INFO = 2,
		LEVEL_DEBUG = 1,
		LEVEL_NOTSET = 0
	} LogLevel;

	typedef Hash<LogLevel> LogLevels;

	//////////////////////////  static //////////////////////////

	/**
	* Инициализирует Logger по данному файлу конфигурации.
	* Инициализация происходит только если logger не был проинициализирован до этого.
	* Если файл конфигурации не найден, или произошла какая-нибудь ошибка при
	* инициализации, то logger инициализируется параметрами по умолчанию
	* (файл smsc.log в текущей директории, уровень DEBUG)
	* \param configFileName имя файла конфигурации logger
	**/
	static void Init(const char * const configFileName);
	static void Init();


	/*!
	* Деинициализирует logger. После этого его можно снова инициализировать.
	*/
	static void Shutdown();

	static inline bool isInitialized() 
	{
		MutexGuard guard(static_mutex);
		return initialized;
	}

	/**
	* retrieves smsc::logger::Logger instance for given category name
	* \param logCategoryName Category name to retrieve
	* \return smsc::logger::Logger logger
	**/
	static Logger * getInstance(const char * const logCategoryName);

	/**
	* Возвращает map CatName -> DebugLevel.
	*/
	static const LogLevels * getLogLevels();
	static void setLogLevels(const LogLevels & newLogCats);
	static const char * const getLogLevel(const LogLevel level) throw();
	static const LogLevel getLogLevel(const char * const logLevelName) throw();

	//////////////////////////  instance //////////////////////////

	inline const char * const getName() const throw() {
		return name;
	}
  
  inline void setAppender(Appender * newAppender) {
    MutexGuard guard(mutex);
    appender = newAppender;
  }

	inline bool isLogLevelEnabled(const LogLevel _logLevel) throw() {
		MutexGuard guard(mutex);
		return this->logLevel <= _logLevel;
	}

	inline bool isFatalEnabled() throw() 
	{ 
		return isLogLevelEnabled(Logger::LEVEL_FATAL);
	};

	inline bool isErrorEnabled() throw() 
	{ 
		return isLogLevelEnabled(Logger::LEVEL_ERROR);
	};

	inline bool isWarnEnabled() throw() 
	{ 
		return isLogLevelEnabled(Logger::LEVEL_WARN);
	};

	inline bool isInfoEnabled() throw() 
	{ 
		return isLogLevelEnabled(Logger::LEVEL_INFO);
	};

	inline bool isDebugEnabled() throw() 
	{ 
		return isLogLevelEnabled(Logger::LEVEL_DEBUG);
	};

	inline const LogLevel getLogLevel() throw() 
	{
		MutexGuard guard(mutex);
		return logLevel;
	}

	/** 
	* Log a message.
	* @param message String to write in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	inline void log(const LogLevel _logLevel, const std::string &message) throw()
	{
		if (isLogLevelEnabled(_logLevel))
			log_(logLevel, message);
	}

	/** 
	* Log a message with debug priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	inline void log(const LogLevel _logLevel, const char * const stringFormat, ...) throw()
	{
		if (isLogLevelEnabled(_logLevel))
		{
			va_list args;
			va_start(args, stringFormat);
			logva_(_logLevel, stringFormat, args);
			va_end(args);
		}
	}

	/** 
	* Log a message with debug priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	inline void logva(const LogLevel _logLevel, const char * const stringFormat, va_list args) throw()
	{
		if (isLogLevelEnabled(_logLevel))
			logva_(logLevel, stringFormat, args);
	}

	/** 
	* Log a message.
	* Do not checks loglevel.
	* @param message String to write in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void log_(const LogLevel logLevel, const std::string &message) throw();

	/** 
	* Log a message with debug priority.
	* Do not checks loglevel.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void log_(const LogLevel logLevel, const char * const stringFormat, ...) throw();

	/** 
	* Log a message with debug priority.
	* Do not checks loglevel.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void logva_(const LogLevel logLevel, const char * const stringFormat, va_list args) throw();

#ifdef SMSC_DEBUG
	static void printDebugInfo();
#endif //SMSC_DEBUG

private:
	static bool initialized;
	typedef smsc::core::buffers::Hash<Logger*> LoggersHash;
	typedef smsc::core::buffers::Hash<Appender*> AppendersHash;
	static LoggersHash loggers;
	static AppendersHash appenders;
	static LogLevels logLevels;
	static Properties cats2appenders;
	static Mutex static_mutex;

	static Logger * getInstanceInternal(const char * const logCategoryName);
	static void clear() throw(Exception);
	static void configure(const char * const configFileName) throw (Exception);
	static void configureAppenders(const ConfigReader & properties) throw (Exception);
	static void configureCatAppenders(const ConfigReader & properties) throw (Exception);
	static void configureRoot(const ConfigReader & config) throw (Exception);
	static LogLevel findDebugLevel(const char * const name);
	static Appender* findAppenderByCat(const char * const name);

	Logger(const char * const logCategoryName, const LogLevel logLevel, Appender * const appender);
	LogLevel logLevel;
	const char* const name;
	Appender * appender;
	Mutex mutex;

	// disable copying
	Logger(const Logger & copy);
	Logger & operator=(const Logger & other);
	inline void setLogLevel(const LogLevel level) throw() 
	{ 
		MutexGuard guard(mutex); 
		logLevel = level;
	}
};

}
}
#endif // ifndef SMSC_LOGGER_H_INCLUDED_C3A87A6B
