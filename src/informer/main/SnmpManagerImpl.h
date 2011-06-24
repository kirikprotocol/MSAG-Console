#ifndef _INFORMER_SNMPMANAGERIMPL_H
#define _INFORMER_SNMPMANAGERIMPL_H

#include "core/threads/Thread.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "logger/Logger.h"
#include "informer/snmp/SnmpManager.h"

namespace eyeline {
namespace informer {

class SnmpManagerImpl : public SnmpManager, protected smsc::core::threads::Thread
{
public:
    SnmpManagerImpl();
    virtual ~SnmpManagerImpl();
    void init( const std::string& snmpSock = "" );
    void start();
    virtual void sendTrap( SnmpTrap::Type     type,
                           SnmpTrap::Severity severity,
                           const char*        category,
                           const char*        objid,
                           const char*        message ); // may throw

private:
    void stop();
    virtual int Execute();

private:
    smsc::logger::Logger* log_;
    smsc::core::buffers::FastMTQueue< SnmpTrap* > queue_;
    bool                  stopping_;
};

}
}

#endif /* INFORMERSNMP_H */
