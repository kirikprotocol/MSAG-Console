#ifndef SCAG_SESSIONS_SESSION
#define SCAG_SESSIONS_SESSION

#include <inttypes.h>

#include <scag/util/properties/Properties.h>
#include <list>
#include <core/synchronization/EventMonitor.hpp>
#include <logger/Logger.h>
#include <scag/bill/BillingManager.h>
#include <core/buffers/XHash.hpp>

#include <scag/util/sms/HashUtil.h>
#include <scag/transport/SCAGCommand.h>

#include <sms/sms_serializer.h>
//#include "core/buffers/TmpBuf.hpp"

namespace scag { namespace sessions 
{
    using namespace smsc::sms::BufOps;
//    using namespace smsc::core::buffers;

    using smsc::logger::Logger;
    using namespace scag::util::properties;
    using namespace smsc::sms;
    using smsc::core::synchronization::EventMonitor;

    using namespace scag::transport;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;
    using namespace scag::bill;


    typedef smsc::core::buffers::TmpBuf<char,2048> SessionBuffer;

    struct CSessionKey
    {
        int16_t             USR; // User Session Reference
        smsc::sms::Address  abonentAddr;

        bool operator ==(const CSessionKey& sk) const 
        {
            return ((this->abonentAddr == sk.abonentAddr) && (this->USR == sk.USR));
        }
    };    

    struct COperationKey
    {
        smsc::sms::Address destAddress;
        int key;

        bool operator == (const COperationKey& operationKey) const
        { 
            return ((this->destAddress == operationKey.destAddress)&&(this->key == operationKey.key));
        }

    };


    class XOperationHashFunc {
    public:
        static uint32_t CalcHash(const COperationKey& key) { return XAddrHashFunc::CalcHash(key.destAddress);}
    };


    class SessionOwner
    {
    public:
        virtual void startTimer(CSessionKey key,time_t deadLine) = 0;
    };


    class PendingOperation
    {
    public:
        uint8_t type;
        time_t validityTime;
    };

    class Operation : public PendingOperation
    {
        Logger * logger;
        Operation(const Operation& operation);
        std::list <int> BillList;
    public:
        void attachBill(int BillId); 
        void detachBill(int BillId);
        void rollbackAll();
        ~Operation() {rollbackAll();}
        Operation() :logger(0) {logger = Logger::getInstance("scag.re");};
    };


    typedef XHash<COperationKey,Operation*,XOperationHashFunc> COperationsHash;


    class Session : public PropertyManager
    {

        Logger * logger;
        std::list<PendingOperation> PendingOperationList;
        COperationsHash OperationHash;
        SessionOwner * Owner;
        Operation * m_pCurrentOperation;
        COperationKey currentOperationKey;
        bool needReleaseCurrentOperation;

        CSessionKey             m_SessionKey;
        time_t                  lastAccessTime;
        bool                    bChanged, bDestroy;
        EventMonitor            accessMonitor;
        int                     accessCount;

        Hash<AdapterProperty *> PropertyHash;
        
    public:

        Session(const CSessionKey& key);
        virtual ~Session();
        
        CSessionKey getSessionKey() const  { return m_SessionKey; }
        inline time_t getLastAccessTime() const { return lastAccessTime; }
        inline bool isChanged() const { return bChanged; }
        
        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);

        void setOwner(SessionOwner * _Owner) { Owner = _Owner;}
        bool hasOperations();
        void expireOperation(time_t currentTime);
        bool startOperation(SCAGCommand& cmd);
        void releaseOperation();

        void addPendingOperation(PendingOperation pendingOperation);
        void abort();
        Operation * GetCurrentOperation() const;
        time_t Session::getWakeUpTime();
        void Serialize(SessionBuffer& buff);
        void Deserialize(SessionBuffer& buff);
    };

}}

#endif // SCAG_SESSIONS_SESSION

/*
SessionGuard sg = store.GetSession(key);
Session* s = sg.getSession();
if (!s) {

}
*/
