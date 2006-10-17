#ident "$Id$"
#ifndef __SMSC_SNMP_SCTP_SCTPMON_HPP__
#define __SMSC_SNMP_SCTP_SCTPMON_HPP__

#include <time.h>
#include <inttypes.h>

namespace smsc{
namespace snmp{
namespace sctp{

struct AssociationChangeEvent{
  uint16_t said;
  uint8_t state;
  time_t time;
};

class AssociationChangeListener{
  public:
    virtual void associationChange(AssociationChangeEvent event) = 0;
};


class SctpMonitor{
  public:
    static SctpMonitor* instance();
    int  run();
    void stop();
    void addAssociationChangeListener(AssociationChangeListener* listener);
    void removeAssociationChangeListener();
    void startAllAssociations();
    SctpMonitor();
};

}//namespace sctp
}//namespace snmp
}//namespace smsc
#endif /* __SMSC_SNMP_SCTP_SCTPMON_HPP__ */
