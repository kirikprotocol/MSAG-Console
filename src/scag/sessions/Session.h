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


    class SessionBuffer : public smsc::sms::BufOps::SmsBuffer
    {
    friend class Session;
    protected:
        SessionBuffer& operator >> (std::string& str)
        {
            uint8_t len;
            this->Read((char*)&len,1);
            char scb[256];

            if (len>255)throw smsc::util::Exception("Attempt to read %d byte in buffer with size %d",(int)len,255);

            this->Read(scb,len);
            scb[len] = 0;

            str = scb;
            return *this;
        };       
    public:
       SessionBuffer() : smsc::sms::BufOps::SmsBuffer(2048) {}

    };


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
        friend class Session;
 //       friend class Comparator;

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



        void SerializeProperty(SessionBuffer& buff);
        void SerializeOperations(SessionBuffer& buff);
        void SerializePendingOperations(SessionBuffer& buff);

        void DeserializeProperty(SessionBuffer& buff);
        void DeserializeOperations(SessionBuffer& buff);
        void DeserializePendingOperations(SessionBuffer& buff);
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

        friend class Comparator;
    };
/* 
    class Comparator
    {
        bool compare_properties(Session * s1,Session * s2)
        {
            if (s1->PropertyHash.GetCount() != s2->PropertyHash.GetCount()) return false;

            char * key;
            AdapterProperty * value = 0;

            s1->PropertyHash.First();
            for (Hash <AdapterProperty *>::Iterator it = s1->PropertyHash.getIterator(); it.Next(key, value);)
                if (!s2->PropertyHash.Exists(value->GetName().c_str())) return false;

            return true;
        }

        bool compare_operations(Session * s1,Session * s2)
        {
            Operation * operation;
            COperationKey key;

            s1->OperationHash.First();
            XHash <COperationKey,Operation *,XOperationHashFunc>::Iterator it = s1->OperationHash.getIterator();

            if (s1->OperationHash.Count() != s2->OperationHash.Count()) return false;

            for (;it.Next(key, operation);)
            {              
                if (!s2->OperationHash.Exists(key)) return false;
                Operation * op2 = s2->OperationHash.Get(key);

                if (operation->BillList.size() != op2->BillList.size()) return false;

                std::list<int>::iterator billIt2 = op2->BillList.begin();
                for (std::list<int>::iterator billIt = operation->BillList.begin();billIt!=operation->BillList.end(); ++billIt)
                {
                    if ((*billIt)!=(*billIt2)) return false;
                    ++billIt2;
                }

                if (operation->type != op2->type) return false;
                if (operation->validityTime != op2->validityTime) return false;
                
            }
            return true;
        }


        bool compare_pending(Session * s1,Session * s2)
        {
            std::list<PendingOperation>::iterator it;
            std::list<PendingOperation>::iterator it2;

            if (s1->PendingOperationList.size() != s2->PendingOperationList.size()) return false;

            it2 = s2->PendingOperationList.begin();
            for (it = s1->PendingOperationList.begin(); it!=s1->PendingOperationList.end(); ++it)
            {
                if (it->type != it2->type) return false;
                if (it->validityTime != it2->validityTime) return false;

                ++it2;
            }
            return true;
        }


    public:
        bool compare(Session * s1,Session * s2)
        {
            if (!(compare_properties(s1,s2)&&compare_operations(s1,s2)&&(compare_pending(s1,s2)))) return false;

            return (s1->currentOperationKey == s2->currentOperationKey);

            bool hasOp1 = (s1->m_pCurrentOperation != 0);
            bool hasOp2 = (s2->m_pCurrentOperation != 0);

            if (hasOp1 != hasOp2) return false;

            if (s1->m_pCurrentOperation != 0)
            {
                if (s1->currentOperationKey == s2->currentOperationKey); else return false;
            }

            if (s1->needReleaseCurrentOperation != s2->needReleaseCurrentOperation) return false;
            if (s1->m_SessionKey == s2->m_SessionKey); return false;

            if (s1->lastAccessTime != s2->lastAccessTime) return false;

            return true;

        }
    };

}}
  */
#endif // SCAG_SESSIONS_SESSION

/*
SessionGuard sg = store.GetSession(key);
Session* s = sg.getSession();
if (!s) {

}
*/
