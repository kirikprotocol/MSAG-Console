#ifndef SMSC_LOGGER_H_INCLUDED_C3A87A6B
#define SMSC_LOGGER_H_INCLUDED_C3A87A6B

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <core/buffers/Hash.hpp>
#include <core/synchronization/Mutex.hpp>
#include <util/Exception.hpp>
#include <util/Properties.h>

#include "logger/Appender.h"
#include "logger/additional/ConfigReader.h"


#ifndef _WIN32
#define smsc_log_fatal(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isFatalEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_FATAL, __VA_ARGS__)
#define smsc_log_error(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isErrorEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_ERROR, __VA_ARGS__)
#define smsc_log_warn(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isWarnEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_WARN, __VA_ARGS__)
#define smsc_log_info(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isInfoEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_INFO, __VA_ARGS__)
#define smsc_log_debug(logger_param_blahbalh_balh,...) if (!logger_param_blahbalh_balh->isDebugEnabled()) ;else logger_param_blahbalh_balh->log_(smsc::logger::Logger::LEVEL_DEBUG, __VA_ARGS__)
#endif

// using namespace smsc::core::buffers;
// using namespace smsc::core::synchronization;
// using namespace smsc::util;

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

  typedef smsc::core::buffers::Hash<LogLevel> LogLevels;

  //////////////////////////  static //////////////////////////

  /*!
  * ���������������� logger. ����� ����� ��� ����� ����� ����������������.
  */
  static void Shutdown();

  static inline bool isInitialized()
  {
      // smsc::core::synchronization::MutexGuard guard(static_mutex);
      return initialized;
  }

  static void Init();

  /// May be used for tests.  Create one appender into stderr,
  /// with default level of 'level'.
  static void initForTest( LogLevel level );

  static void Reload();

  static void Store();
  
  /**
  * retrieves smsc::logger::Logger instance for given category name
  * \param logCategoryName Category name to retrieve
  * \return smsc::logger::Logger logger
  **/
  static Logger * getInstance(const char * const logCategoryName);

  /**
  * ���������� map CatName -> DebugLevel.
  */
  static const LogLevels * getLogLevels();
  static void setLogLevels(const LogLevels & newLogCats);
  static const char * const getLogLevel(const LogLevel level) throw();
  static const LogLevel getLogLevel(const char * const logLevelName) throw();

  //////////////////////////  instance //////////////////////////

  inline const char * const getName() const throw() {
    return name;
  }

  // NOTE: this method is not safe
  inline void setAppender(Appender * newAppender) {
     // NOTE: we don't need mutex to change a pointer
     // smsc::core::synchronization::MutexGuard guard(mutex);
     appender = newAppender;
  }

  inline bool isLogLevelEnabled(const LogLevel _logLevel) throw() {
    //smsc::core::synchronization::MutexGuard guard(mutex);
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
    //smsc::core::synchronization::MutexGuard guard(mutex);
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
      timeval tv;
      timedConfigReload(tv.tv_sec);
      gettimeofday(&tv,0);
      logva_(tv,_logLevel, stringFormat, args);
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
    {
      timeval tv;
      gettimeofday(&tv,0);
      logva_(tv,logLevel, stringFormat, args);
    }
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
  void logva_(timeval tv,const LogLevel logLevel, const char * const stringFormat, va_list args) throw();

#ifdef SMSC_DEBUG
  static void printDebugInfo();
#endif //SMSC_DEBUG

protected:
  static void Init(const char * const configFileName);

private:
  static bool initialized;
  typedef smsc::core::buffers::Hash<Logger*> LoggersHash;
  typedef smsc::core::buffers::Hash<Appender*> AppendersHash;
  static LoggersHash loggers;
  static AppendersHash appenders;
  static LogLevels logLevels;
  static smsc::util::Properties cats2appenders;
  static smsc::core::synchronization::Mutex static_mutex;
  static ConfigReader configReader;

  static Logger * getInstanceInternal(const char * const logCategoryName);
  static void clear() throw(smsc::util::Exception);
  
  static void configure(const char * const configFileName) throw (smsc::util::Exception);
  static void reconfigure(const char * const configFileName) throw (smsc::util::Exception);
  static void storeConfig(const char * const configFileName) throw (smsc::util::Exception);

  static void configureAppenders(const ConfigReader & properties) throw (smsc::util::Exception);
  static void configureCatAppenders(const ConfigReader & properties) throw (smsc::util::Exception);
  static void configureRoot(const ConfigReader & config) throw (smsc::util::Exception);

  static LogLevel findDebugLevel(const char * const name);
  static Appender* findAppenderByCat(const char * const name);

  static uint32_t reloadConfigInterval;
  static time_t lastReloadConfigCheck;
  static time_t lastConfigMTime;
  
  static void timedConfigReload(time_t now)
  {
    if(reloadConfigInterval && time_t(lastReloadConfigCheck + reloadConfigInterval) < now)
    {
        smsc::core::synchronization::MutexGuard guard(static_mutex);
        if(time_t(lastReloadConfigCheck + reloadConfigInterval) >= time(NULL)) return;
        struct ::stat st;
        const char * logFileName = getenv("SMSC_LOGGER_PROPERTIES");
        if(!logFileName) logFileName = "logger.properties";
        if(::stat(logFileName, &st)) return;
        if(st.st_mtime != lastConfigMTime)
        {
          smsc::logger::Logger::reconfigure(logFileName);
          lastConfigMTime=st.st_mtime;
        }
        lastReloadConfigCheck = time(NULL);
    }
  }
  
  Logger(const char * const logCategoryName, const LogLevel logLevel, Appender * const appender);
#ifdef NEWLOGGER
  unsigned prefixLength; // a length required by appender (including trailing \0)
#endif
  LogLevel logLevel;
  const char* const name;
  Appender * appender;   // NOTE: appender is not safe to be changed!
  // smsc::core::synchronization::Mutex mutex;

  // disable copying
  Logger(const Logger & copy);
  ~Logger();
  Logger & operator=(const Logger & other);
  inline void setLogLevel(const LogLevel level) throw()
  {
    // smsc::core::synchronization::MutexGuard guard(mutex);
    logLevel = level;
  }
};

}
}

#ifdef _WIN32
template <class T>
inline void smsc_log_fatal(T* l,const char* stringFormat,...)
{
  va_list args;
  va_start(args, stringFormat);
  l->logva(smsc::logger::Logger::LEVEL_FATAL, stringFormat,args);
  va_end(args);
}
template <class T>
inline void smsc_log_error(T* l,const char* stringFormat,...)
{
  va_list args;
  va_start(args, stringFormat);
  l->logva(smsc::logger::Logger::LEVEL_ERROR, stringFormat,args);
  va_end(args);
}
template <class T>
inline void smsc_log_warn(T* l,const char* stringFormat,...)
{
  va_list args;
  va_start(args, stringFormat);
  l->logva(smsc::logger::Logger::LEVEL_WARN, stringFormat,args);
  va_end(args);
}
template <class T>
inline void smsc_log_info(T* l,const char* stringFormat,...)
{
  va_list args;
  va_start(args, stringFormat);
  l->logva(smsc::logger::Logger::LEVEL_INFO, stringFormat,args);
  va_end(args);
}
template <class T>
inline void smsc_log_debug(T* l,const char* stringFormat,...)
{
  va_list args;
  va_start(args, stringFormat);
  l->logva(smsc::logger::Logger::LEVEL_DEBUG, stringFormat,args);
  va_end(args);
}
#endif

#endif // ifndef SMSC_LOGGER_H_INCLUDED_C3A87A6B
