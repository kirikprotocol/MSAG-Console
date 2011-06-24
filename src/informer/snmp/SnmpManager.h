#ifndef _INFORMER_SNMPMANAGER_H
#define _INFORMER_SNMPMANAGER_H

#include "SnmpTrap.h"

namespace eyeline {
namespace informer {

class SnmpManager
{
public:
    virtual ~SnmpManager() {}
    virtual void sendTrap( SnmpTrap::Type     type,
                           SnmpTrap::Severity severity,
                           const char*        category,
                           const char*        objid,
                           const char*        message ) = 0; // may throw
};

}
}

#endif /* INFORMERSNMP_H */
