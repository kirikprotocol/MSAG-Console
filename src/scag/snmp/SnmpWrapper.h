#ifndef _SCAG_SNMP_SNMPWRAPPER_H
#define _SCAG_SNMP_SNMPWRAPPER_H

#include <string>
#include "logger/Logger.h"
#include "TrapRecord.h"

namespace scag2 {
namespace snmp {

/**
 * A wrapper around net-snmp library.
 */
class SnmpWrapper 
{
public:
    SnmpWrapper( const std::string& socket = "" );
    ~SnmpWrapper();
    void sendTrap( const TrapRecord& rec );

private:
    smsc::logger::Logger* log_;
};

}
}

#endif /* _SCAG_SNMP_SNMPWRAPPER_H */
