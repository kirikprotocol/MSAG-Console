#ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
#define LOG4CPPINIT_H_INCLUDED_C3A87A6B

#include <string>
#include <stdarg.h>
#include <map>

#ifdef LOGGER_LIB_LOG4CPP
#include <log4cpp/Category.hh>
#else
#include <log4cplus/logger.h>
#endif

namespace smsc {
namespace logger {

class Logger
{
public:
	typedef enum {
		LEVEL_FATAL,
		LEVEL_ERROR,
		LEVEL_WARN,
		LEVEL_INFO,
		LEVEL_DEBUG,
		LEVEL_NOTSET
	} LogLevel;

	typedef std::map<std::string, LogLevel> LogCats;
	/**
	* retrieves smsc::logger::Logger instance for given category name
	* \param logCategoryName Category name to retrieve
	* \return smsc::logger::Logger logger
	**/
	static Logger getInstance(const std::string &logCategoryName);

	/**
	* retrieves smsc::logger::Logger instance for given category name
	* \param logCategoryName Category name to retrieve
	* \return smsc::logger::Logger logger
	**/
	static Logger getInstance(const char * const logCategoryName);

	/**
	* Инициализирует Logger по данному файлу конфигурации.
	* Инициализация происходит только если logger не был проинициализирован до этого.
	* Если файл конфигурации не найден, или произошла какая-нибудь ошибка при
	* инициализации, то logger инициализируется параметрами по умолчанию
	* (файл smsc.log в текущей директории, уровень DEBUG)
	* \param configFileName имя файла конфигурации logger
	**/
	static void Init(const std::string & configFileName);

	/*!
	* Деинициализирует logger. После этого его можно снова инициализировать.
	*/
	static void Shutdown();

	static LogCats* getCurrentCategories();
	static void setCategoryLogLevel(const char * const catName, Logger::LogLevel logLevel);
	static const char * const getLogLevelName(const LogLevel level) throw();
	static const LogLevel getLogLevel(const char * const logLevelName) throw();


	Logger(const Logger& copy);
	/** 
    * Returns true if the chained log level of the Logger is equal to
    * or higher than given priority.
    * @param logLevel The log level to compare with.
    * @returns whether logging is enable for this log level.
    **/
	bool isLogLevelEnabled(const LogLevel logLevel) const throw();

	/** 
	* Log a message.
	* @param message String to write in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void log(const LogLevel logLevel, const std::string &message) throw();

	/** 
	* Log a message with debug priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void log(const LogLevel logLevel, const char * const stringFormat, ...) throw();

	/** 
	* Log a message with debug priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void logva(const LogLevel logLevel, const char * const stringFormat, va_list args) throw();

	/** 
	* Log a message with fatal priority. 
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void fatal(const char* stringFormat, ...) throw();

	/** 
	* Log a message with fatal priority.
	* @param message string to write in the log file
	**/  
	void fatal(const std::string& message) throw();

	/**
	* Return true if the Category will log messages with priority FATAL.
	* @returns Whether the Category will log.
	**/ 
	inline bool isFatalEnabled() const throw() { 
		return isLogLevelEnabled(Logger::LEVEL_FATAL);
	};



	/** 
	* Log a message with error priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void error(const char* stringFormat, ...) throw();

	/** 
	* Log a message with error priority.
	* @param message string to write in the log file
	**/  
	void error(const std::string& message) throw();

	/**
	* Return true if the Category will log messages with priority ERROR.
	* @returns Whether the Category will log.
	**/ 
	inline bool isErrorEnabled() const throw() { 
		return isLogLevelEnabled(Logger::LEVEL_ERROR);
	};


	/** 
	* Log a message with warn priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void warn(const char* stringFormat, ...) throw();

	/** 
	* Log a message with warn priority.
	* @param message string to write in the log file
	**/  
	void warn(const std::string& message) throw();

	/**
	* Return true if the Category will log messages with priority WARN.
	* @returns Whether the Category will log.
	**/ 
	inline bool isWarnEnabled() const throw() { 
		return isLogLevelEnabled(Logger::LEVEL_WARN);
	};


	/** 
	* Log a message with info priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void info(const char* stringFormat, ...) throw();

	/** 
	* Log a message with info priority.
	* @param message string to write in the log file
	**/  
	void info(const std::string& message) throw();

	/**
	* Return true if the Category will log messages with priority INFO.
	* @returns Whether the Category will log.
	**/ 
	inline bool isInfoEnabled() const throw() { 
		return isLogLevelEnabled(Logger::LEVEL_INFO);
	};


	/** 
	* Log a message with debug priority.
	* @param stringFormat Format specifier for the string to write 
	* in the log file.
	* @param ... The arguments for stringFormat 
	**/  
	void debug(const char* stringFormat, ...) throw();

	/** 
	* Log a message with debug priority.
	* @param message string to write in the log file
	**/  
	void debug(const std::string& message) throw();

	/**
	* Return true if the Category will log messages with priority DEBUG.
	* @returns Whether the Category will log.
	**/ 
	inline bool isDebugEnabled() const throw() { 
		return isLogLevelEnabled(Logger::LEVEL_DEBUG);
	};


protected:
	static bool isInitialized;

	Logger(const std::string & logCategoryName);
	Logger(const char * const logCategoryName);

private:
#ifdef LOGGER_LIB_LOG4CPP
	log4cpp::Category& logger;
#else
	log4cplus::Logger logger;
#endif
};

}
}
#endif // ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
