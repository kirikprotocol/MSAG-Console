#include "logger/Logger.h"

#include <stdio.h>
#ifndef _WIN32
#include <strings.h>
#else
#include <string.h>
#endif

#include "logger/additional/StderrAppender.h"
#include "logger/additional/RollingFileAppender.h"
#include "logger/additional/RollingIntervalAppender.h"
#include "util/cstrings.h"
#include "util/findConfigFile.h"
#include "core/synchronization/Mutex.hpp"

#ifdef SMSC_DEBUG
#include <iostream>
#endif //SMSC_DEBUG


namespace smsc {
namespace logger {

using namespace smsc::util;
using namespace smsc::core::buffers;
using smsc::core::synchronization::MutexGuard;
using smsc::core::synchronization::Mutex;

////////////////////// internal helpers
const char logChars[] = {'*', 'D', 'I', 'W', 'E', 'F'};
const char* const logNames[] = {"NOTSET", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

char * vform(const char* format, va_list args,char* buf,size_t bufsize)
{
  size_t size = bufsize;
  char* buffer = buf;//new char[size];

  try
  {

    while (1) {
      int n = ::vsnprintf(buffer, size, format, args);
  
      // If that worked, return a string.
      if ((n > -1) && (static_cast<size_t>(n) < size)) {
        return buffer;
      }
  
      // Else try again with more space.
      size = (n > -1) ?
        n + 1 :   // ISO/IEC 9899:1999
      size * 2; // twice the old size
  
      if(buffer!=buf)delete [] buffer;
      buffer = new char[size];
    }
  } catch(std::exception& e)
  {
    sprintf(buf,"out of memory");
    return buf;
  }
}

inline void __loggerError(const char * const message)
{
  fprintf(stderr, "Logger error: %s\naborting...", message);
  ::abort();
}

////////////////////// static fields
bool Logger::initialized = false;
Logger::LoggersHash   Logger::loggers;
Logger::AppendersHash Logger::appenders;
Logger::LogLevels     Logger::logLevels;
Properties            Logger::cats2appenders;
#ifdef CHECKCONTENTION
Mutex                 Logger::static_mutex(MTXFORCENOCHECK);
#else
Mutex                 Logger::static_mutex;
#endif
uint32_t              Logger::reloadConfigInterval = 0;
time_t                Logger::lastReloadConfigCheck = time(NULL);
time_t                Logger::lastConfigMTime = 0;
ConfigReader          Logger::configReader;

Logger *_trace_cat = NULL;
Logger *_map_cat = NULL;
Logger *_mapmsg_cat = NULL;
Logger *_mapdlg_cat = NULL;
Logger *_mapproxy_cat = NULL;
Logger *_sms_err_cat = NULL;

////////////////////// static public methods
void Logger::Init(const char * const configFileName)
{
  MutexGuard guard(static_mutex);
  if(initialized) {
    __loggerError("Init: logger already initialized");
  }
  try {
    clear();
    configure(configFileName);
  } catch (const Exception & e) {
    __loggerError(e.what());
  }
}


void Logger::Init()
{
  char * logFileName = getenv("SMSC_LOGGER_PROPERTIES");
  if (logFileName)
    smsc::logger::Logger::Init(logFileName);
  else
    smsc::logger::Logger::Init("logger.properties");
}


void Logger::initForTest( LogLevel level )
{
  MutexGuard guard(static_mutex);
  if(initialized) {
    __loggerError("Init: logger already initialized");
  }
  try {
    clear();
    // configure(configFileName);
      Properties props;
      if ( unsigned(level) >= sizeof(logNames)/sizeof(const char*) ) {
          throw Exception("unknown level %d",int(level));
      }
      char buf[30];
      sprintf(buf,"%s, default",logNames[unsigned(level)]);
      props.Insert("root",cStringCopy(buf));
      props.Insert("appender.default.stderr.name", cStringCopy("stderr"));
      reloadConfigInterval = 0;
      lastConfigMTime = 0;
      configReader.init(props);
      configureAppenders(configReader);
      configureCatAppenders(configReader);
      configureRoot(configReader);

      initialized = true;
      _trace_cat=getInstanceInternal("trace");
      _map_cat=getInstanceInternal("map");
      _mapmsg_cat=getInstanceInternal("map.msg");
      _mapdlg_cat=getInstanceInternal("map.dialog");
      _mapproxy_cat=getInstanceInternal("map.proxy");
      _sms_err_cat=getInstanceInternal("sms.error");

  } catch (const Exception & e) {
    __loggerError(e.what());
  }
}


void Logger::Reload()
{
  MutexGuard guard(static_mutex);
  if(!initialized)
  {
      __loggerError("setCategoryLogLevel: Logger not initialized");
      return;
  }
  char * logFileName = getenv("SMSC_LOGGER_PROPERTIES");
  smsc::logger::Logger::reconfigure(logFileName ? logFileName : "logger.properties");
}

void Logger::Store()
{
  MutexGuard guard(static_mutex);
  if(!initialized)
  {
      __loggerError("setCategoryLogLevel: Logger not initialized");
      return;
  }
  char * logFileName = getenv("SMSC_LOGGER_PROPERTIES");
  smsc::logger::Logger::storeConfig(logFileName ? logFileName : "logger.properties");
}

void Logger::Shutdown()
{
  MutexGuard guard(static_mutex);
  if (initialized){
    clear();
  }
  else
  {
    __loggerError("Shutdown: logger already shutdowned");
  }
}

Logger * Logger::getInstance(const char * const name)
{
  MutexGuard guard(static_mutex);
  if (!initialized)
  {
    std::string s("getInstance: Logger not initialized. Trying to get category \"");
    s += name;
    s += '"';
    __loggerError(s.c_str());
  }
  return getInstanceInternal(name);
}

Logger * Logger::getInstanceInternal(const char * const name)
{
  if (!loggers.Exists(name))
    loggers.Insert(name, new Logger(name, findDebugLevel(name), findAppenderByCat(name)));
  return loggers[name];
}

const Logger::LogLevels * Logger::getLogLevels()
{
  MutexGuard guard(static_mutex);
  if (!initialized)
    __loggerError("getCurrentLoggers: Logger not initialized");

  std::auto_ptr<LogLevels> ls(new LogLevels());
  char *k;
  LogLevel ll;
  for (LogLevels::Iterator i (& logLevels); i.Next(k, ll); )
  {
    ls->Insert(k, ll);
  }

  Logger * l;
  for (LoggersHash::Iterator i (&loggers); i.Next(k, l); )
  {
    if (!logLevels.Exists(k))
    {
      LogLevel levF = findDebugLevel(k);
      LogLevel levL = l->getLogLevel();
      ls->Insert(k, levF != levL ? levL : LEVEL_NOTSET);
    }
  }

  return ls.release();
}

void Logger::setLogLevels(const Logger::LogLevels & _logLevels)
{
  MutexGuard guard(static_mutex);
  if (initialized) {
    {
      char* k;
      LogLevel l;
      logLevels.Empty();
      for (LogLevels::Iterator i (&_logLevels); i.Next(k, l); )
      {
          if (_logLevels[k] != LEVEL_NOTSET)
              logLevels[k] = l;
          if(!*k)
              configReader.rootLevel.reset(cStringCopy(getLogLevel(l)));
          else
          {
              ConfigReader::CatInfo** ci = configReader.cats.GetPtr(k);
              if(ci) (*ci)->level.reset(cStringCopy(getLogLevel(l)));
          }
      }
    }
    {
      char* k;
      Logger* l;
      for (LoggersHash::Iterator i (& loggers); i.Next(k, l); )
      {
        l->setLogLevel(Logger::findDebugLevel(l->getName()));
      }
    }
  }
  else
    __loggerError("setCategoryLogLevel: Logger not initialized");
}

const char * const Logger::getLogLevel(const LogLevel level) throw()
{
  if (level >= Logger::LEVEL_NOTSET && level <= Logger::LEVEL_FATAL)
    return logNames[level];
  else
    return logNames[Logger::LEVEL_NOTSET];
}

const Logger::LogLevel Logger::getLogLevel(const char * const logLevelName) throw()
{
  for (unsigned i = Logger::LEVEL_NOTSET; i<= Logger::LEVEL_FATAL; i++)
    if (strcmp(logNames[i], logLevelName) == 0)
      return (Logger::LogLevel)i;
  return Logger::LEVEL_NOTSET;
}


////////////////////// static private methods
void truncateLastNamePart(char * str)
{
  char * pos = strrchr(str, '.');
  if (pos != NULL)
    *pos = 0;
  else
    *str = 0;
}

void Logger::clear() throw(Exception)
{
  initialized = false;
  {
    char* key;
    Logger* val;
    for (LoggersHash::Iterator i(&loggers); i.Next(key,val);)
    {
      if (val != NULL)
        delete val;
    }
    loggers.Empty();
  }

  {
    char* key;
    Appender* val;
    for (AppendersHash::Iterator i(&appenders); i.Next(key,val);)
    {
      if (val != NULL)
        delete val;
    }
    appenders.Empty();
  }

  {
    char* key;
    const char* val;
    for (Properties::Iterator i(&cats2appenders); i.Next(key,val);)
    {
      if (val != NULL)
        delete val;
    }
    cats2appenders.Empty();
  }

  _trace_cat = NULL;
  _map_cat = NULL;
  _mapmsg_cat = NULL;
  _mapdlg_cat = NULL;
  _mapproxy_cat = NULL;
  _sms_err_cat = NULL;
}

void Logger::configureAppenders(const ConfigReader & configs) throw (Exception)
{
  char * key;
  ConfigReader::AppenderInfo * info;

  for (ConfigReader::AppenderInfos::Iterator i (& configs.appenders); i.Next(key, info); )
  {
    if (strcasecmp(info->type.get(), "stderr") == 0) {
      appenders.Insert(key, new StderrAppender(key));
    }
    else if (strcasecmp(info->type.get(), "file") == 0) {
      appenders.Insert(key, new RollingFileAppender(key, *info->params));
    }
    else if (strcasecmp(info->type.get(), "dayfile") == 0)
      appenders.Insert(key, new RollingIntervalAppender(key, *info->params, ".%04d-%02d-%02d"));
    else if (strcasecmp(info->type.get(), "intervalfile") == 0)
      appenders.Insert(key, new RollingIntervalAppender(key, *info->params));
    else
    {
      std::string msg("Unknown appender type: \"");
      msg += info->type.get();
      msg += "\"";
      __loggerError(msg.c_str());
    }
  }
}

void Logger::configureCatAppenders(const ConfigReader & configs) throw (Exception)
{
  char * key;
  ConfigReader::CatInfo* catInfo;
  for (ConfigReader::CatInfos::Iterator i (& configs.cats); i.Next(key, catInfo); )
  {
    if (catInfo->appender.get() != NULL && strlen(catInfo->appender.get()) > 0)
      cats2appenders.Insert(catInfo->name.get(), cStringCopy(catInfo->appender.get()));
    if (catInfo->level.get() != NULL && strlen(catInfo->level.get()) > 0)
    {
      Logger::LogLevel level = Logger::getLogLevel(catInfo->level.get());
      if (level != Logger::LEVEL_NOTSET)
        logLevels.Insert(catInfo->name.get(), level);
    }
  }
}

void Logger::configureRoot(const ConfigReader & cr) throw (Exception)
{
  if (cr.rootAppender.get() == NULL || strlen(cr.rootAppender.get()) == 0 || !appenders.Exists(cr.rootAppender.get()))
  {
    std::string s = "root appender ";
    if (cr.rootAppender.get() != NULL)
    {
      s += "\"";
      s += cr.rootAppender.get();
      s += "\" ";
    }
    s += "not found";
    __loggerError(s.c_str());
  }
  LogLevel rootLevel = Logger::getLogLevel(cr.rootLevel.get());
  if (rootLevel == LEVEL_NOTSET)
    rootLevel = LEVEL_DEBUG;
  logLevels.Insert("", rootLevel);
  cats2appenders.Insert("", cStringCopy(cr.rootAppender.get()));
}

void Logger::configure(const char * const configFileName) throw (Exception)
{
  Properties props(configFileName);

  if(props.Exists("configReloadInterval"))
    reloadConfigInterval = atoi(props["configReloadInterval"]);

  struct ::stat st={0,};
  ::stat((const char*)configFileName, &st);
  lastConfigMTime=st.st_mtime;


  configReader.init(props);
  configureAppenders(configReader);
  configureCatAppenders(configReader);
  configureRoot(configReader);

  initialized = true;
  _trace_cat=getInstanceInternal("trace");
  _map_cat=getInstanceInternal("map");
  _mapmsg_cat=getInstanceInternal("map.msg");
  _mapdlg_cat=getInstanceInternal("map.dialog");
  _mapproxy_cat=getInstanceInternal("map.proxy");
  _sms_err_cat=getInstanceInternal("sms.error");
}

void Logger::reconfigure(const char * const configFileName) throw (Exception)
{
  Properties props(configFileName);

  if(props.Exists("configReloadInterval"))
    reloadConfigInterval = atoi(props["configReloadInterval"]);

  configReader.init(props);

  char * key;
  ConfigReader::CatInfo* catInfo;
  for (ConfigReader::CatInfos::Iterator i (& configReader.cats); i.Next(key, catInfo); )
  {
    if (catInfo->level.get() != NULL && strlen(catInfo->level.get()) > 0)
    {
      Logger::LogLevel level = Logger::getLogLevel(catInfo->level.get());
      if(logLevels.Exists(catInfo->name.get()))
          logLevels[catInfo->name.get()] = level;
      else
          logLevels.Insert(catInfo->name.get(), level);
    }
  }
  {
    char* k;
    Logger* l;
    for (LoggersHash::Iterator i (& loggers); i.Next(k, l); )
      l->setLogLevel(Logger::findDebugLevel(l->getName()));
  }
}

void Logger::storeConfig(const char * const configFileName) throw (Exception)
{
    std::string str;
    configReader.serialize(str);
    char* k;
    LogLevel l;
    for (LogLevels::Iterator i (& logLevels); i.Next(k, l); )
    {
        ConfigReader::CatInfo** ci = configReader.cats.GetPtr(k);
        if(!ci && *k)
        {
            str += "cat."; str += k; str += '='; str += getLogLevel(l); str += '\n';
        }
    }

    const char* fn = findConfigFile(configFileName);
    File f;
    f.RWCreate(fn);
    f.Write(str.c_str(), str.length());
}

Logger::LogLevel Logger::findDebugLevel(const char * const name)
{
  std::auto_ptr<char> n(cStringCopy(name));
  while (strlen(n.get()) > 0 && (!logLevels.Exists(n.get())))
    truncateLastNamePart(n.get());

  if (logLevels.Exists(n.get()))
    return logLevels[n.get()];
  else
    return Logger::LEVEL_DEBUG;
}

Appender* Logger::findAppenderByCat(const char * const name)
{
  std::auto_ptr<char> n(cStringCopy(name));
  while (strlen(n.get()) > 0 && !cats2appenders.Exists(n.get()))
    truncateLastNamePart(n.get());

  if (cats2appenders.Exists(n.get()))
  {
    if (appenders.Exists(cats2appenders[n.get()]))
      return appenders[cats2appenders[n.get()]];
    else
    {
      std::string s("Appender \"");
      s += cats2appenders[n.get()];
      s += "\"";
      std::auto_ptr<Appender> a(new StderrAppender(s.c_str()));
      s += " not found";
      __loggerError(s.c_str());
      const char * const an = a->getName();
      appenders.Insert(an, a.release());
      return appenders[an];
    }
  }
  else
  {
    std::string s("Appender for logger \"");
    s += name;
    s += "\"";
    std::auto_ptr<Appender> a(new StderrAppender(s.c_str()));
    s += " not found";
    __loggerError(s.c_str());
    const char * const an = a->getName();
    appenders.Insert(an, a.release());
    return appenders[an];
  }
}


////////////////////// constructors
Logger::Logger(const char * const logCategoryName, const LogLevel logLevel, Appender * const appender)
  :logLevel(logLevel), name(cStringCopy(logCategoryName)), appender(appender), mutex(MTXFORCENOCHECK)
{
}

Logger::~Logger()
{
  MutexGuard guard(mutex);
  delete name;
  appender=0;
  logLevel=LEVEL_NOTSET;
}

Logger::Logger(const Logger& copy)
  :name(cStringCopy("")), appender(NULL)
{
  __loggerError("Logger(const Logger& copy): copying instance of Logger is prohibited");
}

Logger & Logger::operator = (const Logger & other)
{
  __loggerError("operator=: copying instance of Logger is prohibited");
  return *this;
}


////////////////////// public methods
void Logger::log_(const LogLevel _logLevel, const std::string &message) throw()
{
#ifdef NEWLOGGER
  log_(_logLevel,"%s",message.c_str());
#else
  timeval tv;
  gettimeofday(&tv,0);
  timedConfigReload(tv.tv_sec);

  if (isInitialized()) {
    try {
      appender->log(tv,logChars[_logLevel], this->name, message.c_str());
    } catch (...) {
    }
  } else {
    __loggerError("log_: Logger not initialized");
    fprintf(stderr, "%c %s: %s", logChars[_logLevel], this->name, message.c_str());
  }
#endif
}

void Logger::log_(const LogLevel _logLevel, const char * const stringFormat, ...) throw()
{
  timeval tv;
  gettimeofday(&tv,0);
  timedConfigReload(tv.tv_sec);

  va_list args;
  va_start(args, stringFormat);
  logva_(tv,_logLevel, stringFormat, args);
  va_end(args);
}


void Logger::logva_(timeval tv,const LogLevel _logLevel, const char * const stringFormat, va_list args) throw()
{
#ifdef NEWLOGGER
    class BufferGuard {
    public:
        BufferGuard() : buf_(0) {}
        ~BufferGuard() { if (buf_) { delete [] buf_; } }
        void setBuf( char* buf ) {
            if (buf_) { delete [] buf_; }
            buf_ = buf;
        }
    private:
        char* buf_;
    };

    if (isInitialized()) {

        char buf[2048];
        BufferGuard bufguard;

        try {

            size_t bufsize = sizeof(buf);
            char* buffer = buf;
            const size_t prefixLength = appender->logPrefix(buf,bufsize,tv,logChars[_logLevel],this->name);
            bufsize -= prefixLength;
            while (true) {
                int n = ::vsnprintf(buffer+prefixLength, bufsize, stringFormat, args);
                if ( (n>=0) && size_t(n)+2 < bufsize ) {
                    bufsize = size_t(n) + prefixLength; // final size
                    break;
                }
                bufsize = (n>-1) ?
                    size_t(n+5) : // ISO/IEC 9899:1999, and reserve some space at the end
                    bufsize*2+32; // twice the old size
                bufguard.setBuf(buffer = new char[bufsize + prefixLength]);
                memcpy(buffer,buf,prefixLength);
            }
            appender->write(tv,buffer,bufsize);

        } catch (...) {
            fprintf(stderr,"out of memory");
            return;
        }

    } else {
        __loggerError("logva_: Logger not initialized");
        char buf[2048];
        int n = ::snprintf(buf,sizeof(buf),"%c %s: ",logChars[_logLevel],this->name);
        ::vsnprintf(buf+n,sizeof(buf)-n-2,stringFormat,args);
        buf[sizeof(buf)-2] = '\0';
        strcat(buf,"\n");
        fputs(buf,stderr);
    }
#else
  char buf[2048];
  char* msg=vform(stringFormat, args,buf,sizeof(buf));
  std::auto_ptr<char> messageBuf(msg==buf?0:msg);
  if (isInitialized())
  {
    try{
      appender->log(tv,logChars[_logLevel], this->name, msg);
    }catch(...)
    {
    }
  } else
  {
    __loggerError("logva_: Logger not initialized");
    fprintf(stderr, "%c %s: %s", logChars[_logLevel], this->name, msg);
  }
#endif
}

#ifdef SMSC_DEBUG
void Logger::printDebugInfo()
{
  MutexGuard guard(static_mutex);

  std::cout << std::endl << "Debug info:" << std::endl;
  std::cout << "  Initialized: " << Logger::initialized << std::endl;

  {
    std::cout << "  Appenders:" << std::endl;
    char * k;
    Appender * val;
    for (AppendersHash::Iterator i = appenders.getIterator(); i.Next(k, val); )
    {
      std::cout << "    " << k << "->\"" << val->getName() << "\"" << std::endl;
    }
  }

  {
    std::cout << "  LogLevels:" << std::endl;
    char * k;
    LogLevel val;
    for (LogLevels::Iterator i = logLevels.getIterator(); i.Next(k, val); )
    {
      std::cout << "    " << k << "->" << Logger::getLogLevel(val) << std::endl;
    }
  }

  {
    std::cout << "  cats2appenders:" << std::endl;
    char * k;
    const char * val;
    for (Properties::Iterator i = cats2appenders.getIterator(); i.Next(k, val); )
    {
      std::cout << "    " << k << "->" << val << std::endl;
    }
  }

  {
    std::cout << "  Loggers:" << std::endl;
    char * k;
    Logger * val;
    for (LoggersHash::Iterator i = loggers.getIterator(); i.Next(k, val); )
    {
      std::cout << "    " << k << "->\"" << val->getName() << "\" [" << Logger::getLogLevel(val->getLogLevel()) << ", " << (val->appender != NULL ? val->appender->getName() : "null") << "]" << std::endl;
    }
  }
}
#endif //SMSC_DEBUG

}
}
