#ifndef __SMSC_SNMP_SNMPAPPENDER_HPP__
#define __SMSC_SNMP_SNMPAPPENDER_HPP__

#include <log4cpp/Portability.hh>
#include <log4cpp/LayoutAppender.hh>
#include <string>
#include <stdarg.h>

namespace smsc {
  namespace snmp {
    class SnmpAgent;
    using log4cpp::LayoutAppender;
    using log4cpp::LoggingEvent;
    class SnmpAppender : public LayoutAppender {
      public:
        SnmpAppender(const std::string& name, SnmpAgent *agent);
        virtual ~SnmpAppender();
        virtual void close();

      protected:
        virtual void _append(const LoggingEvent& event);
      private:
        SnmpAgent *agent;
    };
  }
}

#endif // __SMSC_SNMP_SNMPAPPENDER_HPP__
