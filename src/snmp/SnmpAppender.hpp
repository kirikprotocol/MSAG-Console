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
	SnmpAppender(const char * const _name, SnmpAgent *_agent)
		: Appender(_name), agent(_agent)
	{}
	virtual ~SnmpAppender() {};
  
 	virtual void log(const char logLevelName, const char * const category, const char * const message) throw();

private:
	SnmpAgent *agent;
};

}
}

#endif // __SMSC_SNMP_SNMPAPPENDER_HPP__
