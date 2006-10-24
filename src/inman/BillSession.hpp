#ident "$Id$"

#ifndef __SMSC_INMAN_BILLING_SESSION__
#define __SMSC_INMAN_BILLING_SESSION__

#include "inman/inman.hpp"
using smsc::inman::AbonentPolicies;
using smsc::inman::INScfCFG;
using smsc::inman::SmsXServiceMap;

#include "inman/common/TimeWatcher.hpp"
using smsc::inman::sync::TimeWatcher;

#include "inman/storage/CDRStorage.hpp"
using smsc::inman::filestore::InBillingFileStorage;

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ConnectListenerITF;

#include "inman/interaction/messages.hpp"
using smsc::inman::interaction::INPPacketAC;

namespace smsc    {
namespace inman   {

typedef enum { BILL_NONE = 0, BILL_ALL, BILL_USSD, BILL_SMS } BILL_MODE;

struct BillingCFG {
    typedef enum { CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2} CDR_MODE;
    AbonentCacheITF *   abCache;
    InBillingFileStorage * bfs;
    TimeWatcher *       tmWatcher;
//billing parameters
    AbonentPolicies * policies;
    BILL_MODE       billMode;
    CDR_MODE        cdrMode;
    std::string     cdrDir;         //location to store CDR files
    long            cdrInterval;    //rolling interval for CDR files
    unsigned short  maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    unsigned short  abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    unsigned short  maxBilling;     //maximum number of Billings per connect

    SS7_CFG         ss7;            //SS7 interaction:
    SmsXServiceMap  smsXMap;        //SMS Extra services iDs and addresses

    BillingCFG()
    {
        abCache = NULL; bfs = NULL; tmWatcher = NULL;
        cdrInterval = 0;
        billMode = smsc::inman::BILL_ALL;
        cdrMode =  CDR_ALL;
        maxTimeout = abtTimeout = maxBilling = ss7.maxDlgId = ss7.capTimeout = 0;
        ss7.own_ssn = ss7.userId = 0;
    }

};

class Billing; //see billing.hpp

//Manages SMSC Requests on given Connect in parallel/asynchronous mode
//(for each request initiates new Billing).
class BillingConnect: public ConnectListenerITF {
public: 
    BillingConnect(BillingCFG * cfg, Connect* conn, Logger * uselog = NULL);
    ~BillingConnect();

    unsigned int    bConnId(void) const { return _bcId; }
    const BillingCFG & getConfig(void) const { return _cfg;}

    //sends command, returns true on success
    bool sendCmd(INPPacketAC* cmd);
    //releases completed Billing, writting CDR if required
    void billingDone(Billing* bill);
    //
    CustomException * connectError(void) const { return _conn ? _conn->hasException() : NULL; }

    //-- ConnectListenerITF interface
    void onCommandReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd);
    //Stops all Billings due to fatal socket error
    void onConnectError(Connect* conn, bool fatal/* = false*/);

protected:
    typedef std::map<unsigned int, Billing*> BillingMap;
    typedef std::list<Billing*> BillingList;

    void cleanUpBills(void);

    Mutex       _mutex;
    Logger*     logger;
    unsigned int _bcId;
    BillingCFG  _cfg;
    BillingMap  workers;
    BillingList corpses;
    Connect*    _conn;
};


} //inman
} //smsc

#endif /* __SMSC_INMAN_BILLING_SESSION__ */

