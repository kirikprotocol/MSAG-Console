#include <stdio.h>
#include <time.h>
#include "SnmpAppender.hpp"
#include "SnmpAgent.hpp"

namespace smsc {
namespace snmp {



#ifdef LOGGER_LIB_LOG4CPP
/*
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
	std::string message(event.message);
	if (agent)
	{
		agent->trap(message);
	}
}
*/
#else
/*
SnmpAppender::SnmpAppender(const char * const name, SnmpAgent *agent)
	:Appender(), agent(agent)
{
}

SnmpAppender::~SnmpAppender()
{
}

void SnmpAppender::close()
{
}

void SnmpAppender::append(const InternalLoggingEvent& event)
{
	std::string message(event.getMessage());
	if (agent)
	{
		agent->trap(message);
	}
}
*/
#endif
}
}
