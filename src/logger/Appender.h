#ifndef SMSC_LOGGER_APPENDER_HEADER
#define SMSC_LOGGER_APPENDER_HEADER

#include <memory>
#include <sys/time.h>

namespace smsc {
namespace logger {

class Appender 
{
protected:
  std::auto_ptr<const char> name;
public:
  Appender(const char * const name);
  virtual ~Appender() {};
#ifdef NEWLOGGER
  virtual unsigned getPrefixLength() throw() = 0;

  // write the formatted message into appender;
  // @param buf -- the formatted message;
  // @param bufsize -- the length of the message not including the trailing '\0'.
  // NOTE: that buf[bufsize] is '\0' initially but appender may change it freely.
  virtual void write(timeval tv, const char logLevelName, const char* category, char* buf, size_t bufsize) throw() = 0;
#else
  virtual void log(timeval tv,const char logLevelName, const char * const category, const char * const message) throw() = 0;
#endif

  inline const char * const getName() const throw() { return name.get(); }
};

}
}

#endif //SMSC_LOGGER_APPENDER_HEADER
