#include <stdio.h>
#include <time.h>
#include "SnmpAppender.hpp"
#include <log4cpp/Category.hh>
#include "SnmpAgent.hpp"
namespace smsc {
  namespace snmp {
    using log4cpp::LoggingEvent;
    SnmpAppender::SnmpAppender(const std::string& name, SnmpAgent *agent) :
      LayoutAppender(name),
      agent(agent)
    {
    }
    
    SnmpAppender::~SnmpAppender()
    {
    }
    void SnmpAppender::close()
    {
    }

    void SnmpAppender::_append(const LoggingEvent& event)
    {
      fprintf(stderr,"snmp appender\n");
        std::string message(event.message);
      //std::string message(_getLayout().format(event));

      if (agent)
      {
        agent->trap(message);
      }
      fprintf(stderr,"snmp appender exit\n");
    }

  }
}
