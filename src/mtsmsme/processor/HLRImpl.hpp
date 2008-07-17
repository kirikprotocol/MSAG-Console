#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_HLRIMPL_HPP__
#define __SMSC_MTSMSME_PROCESSOR_HLRIMPL_HPP__

#include "mtsmsme/processor/Processor.h"
#include "sms/sms.h"
#include <string>
#include <map>

namespace smsc { namespace mtsmsme { namespace processor { class TCO; }}}
namespace smsc{namespace mtsmsme{namespace processor{

using namespace std;
using smsc::sms::Address;
using smsc::mtsmsme::processor::TCO;

//Interface to entity that periodicaly invoke UpdateLocation operation against external(real) HLR
class SubscriberRegistrator: public HLROAM {
  public:
    SubscriberRegistrator(TCO* coordinator);
    virtual void configure(Address& msc, Address& vlr);
    // request to register specified info to HLR on periodical basis specified by period.
    // if 'period' parameter equals zero then register info only once
    virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period);
    // find or create subscriber record in internal database then update it
    // return operation status
    virtual int update(Address& imsi, Address& msisdn, Address& mgt);
    bool lookup(Address& msisdn, Address& imsi);
    void process();
  private:
    map<string,string> hlr;
};

class SubscriberInfo {
  public:
    char imsi[20];   //imsi
    char msisdn[20]; //msidn
    char msc[20];    //serving msc
    char vlr[20];    //serving vlr
};

//Interface to subscriber database
class SubscriberDB {
  public:
    virtual int update(SubscriberInfo* si) = 0; //find or create subscriber record then update it, return operation status
    virtual SubscriberInfo* lookup(const char* msisdn) = 0; //find subscriber record by msisdn
};

class SubscriberInfoInternal: public SubscriberInfo {
  public:

};
class SubscriberDBInMemoryImpl: public SubscriberDB {
  private:
};
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_HLRIMPL_HPP__
