#ifndef __SMSC_SNMP_SNMPAPPENDER_HPP__
#define __SMSC_SNMP_SNMPAPPENDER_HPP__

#include <string>
#include <stdarg.h>

#include "logger/Appender.h"
#include "util/Properties.h"

namespace smsc {
namespace snmp {

class SnmpAgent;

class SnmpAppender : public smsc::logger::Appender {
public:
	SnmpAppender(const char * const _name, SnmpAgent *agent)
		: Appender(_name)
	{}
	virtual ~SnmpAppender() {};
		
private:
	SnmpAgent *agent;
};
#ifdef LOGGER_LIB_LOG4CPP
/*
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
*/
#else
/*
using log4cplus::Appender;
using log4cplus::spi::InternalLoggingEvent;
class SnmpAppender : public Appender {
public:
	SnmpAppender(const std::string& name, SnmpAgent *agent);
	virtual ~SnmpAppender();
	virtual void close();

protected:
	virtual void append(const InternalLoggingEvent& event);
private:
	SnmpAgent *agent;
};
*/
#endif

}
}

#endif // __SMSC_SNMP_SNMPAPPENDER_HPP__
