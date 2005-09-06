#ifndef SCAG_SESSIONS_SESSION
#define SCAG_SESSIONS_SESSION

#include <inttypes.h>

#include <scag/bill/Bill.h>
#include <scag/util/properties/Properties.h>
#include <list>
#include <core/synchronization/EventMonitor.hpp>
#include "sms/sms.h"
#include "scag/transport/SCAGCommand.h"

namespace scag { namespace sessions 
{
    using scag::bill::Bill;

    using namespace scag::util::properties;
    using namespace smsc::sms;
    using smsc::core::synchronization::EventMonitor;
    using scag::transport::SCAGCommand;



    struct CSessionKey
    {
        int16_t             USR; // User Session Reference
        smsc::sms::Address  abonentAddr;

        bool operator ==(const CSessionKey& sk) const 
        {
            return ((this->abonentAddr == sk.abonentAddr) && (this->USR == sk.USR));
        }
    };    

    class SessionOwner
    {
    public:
        virtual void startTimer(CSessionKey key,time_t deadLine) = 0;
        //virtual void stopTimer(CSessionKey key);
    };


    class PendingOperation
    {
    public:
        uint8_t type;
        time_t validityTime;
    };

    class Operation : public PendingOperation
    {
        Operation(const Operation& operation);
    public:
        void attachBill(const Bill& bill);
        void detachBill(const Bill& bill);
        void close(bool commit);
    };


    typedef std::list<Operation*> COperationsList;


    class Session : public PropertyManager
    {
        std::list<PendingOperation> PendingOperationList;
        COperationsList OperationList;
        SessionOwner * Owner;
        Operation * currentOperation;

        CSessionKey             m_SessionKey;
        time_t                  lastAccessTime;
        bool                    bChanged, bDestroy;
        EventMonitor            accessMonitor;
        int                     accessCount;

        Hash<AdapterProperty *> PropertyHash;
        
    public:

        Session();
        virtual ~Session();
        
        CSessionKey getSessionKey() const {
            return m_SessionKey;
        }
        inline time_t getLastAccessTime() const {
            return lastAccessTime;
        }
        inline bool isChanged() const {
            return bChanged;
        }
        
        void Expire();

       
        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);

        void setOwner(SessionOwner& _Owner) {Owner = &_Owner;}
        bool hasOperations() const;
        void expireOperation(time_t currentTime);
        bool startOperation(SCAGCommand& cmd);
        void releaseOperation();

        void addPendingOperation(PendingOperation pendingOperation);
        Operation * GetCurrentOperation() const;
        time_t Session::getWakeUpTime();
    };

}}

#endif // SCAG_SESSIONS_SESSION

/*
SessionGuard sg = store.GetSession(key);
Session* s = sg.getSession();
if (!s) {

}
*/
