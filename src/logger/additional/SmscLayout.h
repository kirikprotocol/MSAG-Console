#ifndef SMSC_LOGGER_LAYOUT_HEADER
#define SMSC_LOGGER_LAYOUT_HEADER

#if 0

#ifndef LOGGER_LIB_LOG4CPP

#include <log4cplus/layout.h>
#include <log4cplus/streams.h>
#include <log4cplus/spi/loggingevent.h>
#include <log4cplus/spi/factory.h>

namespace smsc {
namespace logger {

class SmscLayout: public log4cplus::Layout
{
public:
  SmscLayout()
  {}

  virtual ~SmscLayout(){}

  virtual void formatAndAppend(log4cplus::tostream& output, const log4cplus::spi::InternalLoggingEvent& event);

};

class SmscLayoutFactory: public log4cplus::spi::LayoutFactory {
public:
  SmscLayoutFactory(){}
  virtual ~SmscLayoutFactory(){}

  /**
  * Create a "Layout" object.
  */
  virtual std::auto_ptr<log4cplus::Layout> createObject(const log4cplus::helpers::Properties& props);

  /**
  * Returns the typename of the "Layout" objects this factory creates.
  */
  virtual log4cplus::tstring getTypeName();
};

}
}

#endif //if 0

#endif //LOGGER_LIB_LOG4CPP
#endif //SMSC_LOGGER_LAYOUT_HEADER
