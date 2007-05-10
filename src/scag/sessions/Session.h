#ifndef SCAG_SESSIONS_SESSION
#define SCAG_SESSIONS_SESSION

#include <inttypes.h>

#include <scag/util/properties/Properties.h>
#include <list>
#include <core/synchronization/EventMonitor.hpp>
#include <logger/Logger.h>
#include <core/buffers/XHash.hpp>
#include <core/buffers/IntHash.hpp>

#include <scag/util/sms/HashUtil.h>

#include <sms/sms_serializer.h>
#include <scag/re/RuleStatus.h>
#include "scag/config/sessn/SessionManagerConfig.h"
#include "core/buffers/RefPtr.hpp"

namespace scag { namespace transport {
    class SCAGCommand;
}}

namespace scag { namespace sessions {
    using scag::config::SessionManagerConfig;
    using namespace smsc::sms::BufOps;

    using smsc::logger::Logger;
    using namespace scag::util::properties;
    using namespace smsc::sms;
    using smsc::core::synchronization::EventMonitor;

    using namespace scag::transport;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;
    using scag::re::RuleStatus;

    enum ProtocolForEvent
    {
        PROTOCOL_SMPP_SMS =  1,
        PROTOCOL_SMPP_USSD = 2,
        PROTOCOL_HTTP = 3,
        PROTOCOL_MMS = 4
    };


    enum ICCOperationStatus 
    {
        OPERATION_INITED,
        OPERATION_CONTINUED,
        OPERATION_COMPLETED 
    };
    
    namespace OperationFlags
    {
        const uint32_t SERVICE_INITIATED_USSD_DIALOG = 1;
    }

    class MillisecTime
    {
        timeval timeVal;
        char buff[25];
        bool sync;
    public:
        MillisecTime(timeval tv) : timeVal(tv), sync(false) {}
        const char * toString()
        {
            if (sync) return buff;

            char tmp[4];
            sprintf(tmp,"%ld",timeVal.tv_usec / 1000);
            int len = strlen(tmp);

            while (len < 3)
            {
                tmp[len] = '0';
                len++;
            }
            tmp[len] = 0;

            sprintf(buff, "%ld%s",timeVal.tv_sec,tmp);
            sync = true;
            return buff;
        }
    };


    class SessionBuffer : public smsc::sms::BufOps::SmsBuffer
    {
    friend class Session;
    protected:
        SessionBuffer& operator >> (std::string& str)
        {
            uint8_t len;
            this->Read((char*)&len,1);
            char scb[256];

            if (len>255) throw smsc::util::Exception("Attempt to read %d byte in buffer with size %d",(int)len,255);

            this->Read(scb,len);
            scb[len] = 0;

            str = scb;
            return *this;
        };
    public:
       SessionBuffer() : smsc::sms::BufOps::SmsBuffer(2048) {}
       SessionBuffer(int size):smsc::sms::BufOps::SmsBuffer(size){}
    };


    struct CSessionPrimaryKey
    {
        smsc::sms::Address abonentAddr;
        timeval BornMicrotime;
        
        void toString(std::string& s) const
        {
            s = abonentAddr.toString();
            char buff[128];
            sprintf(buff,"/%ld%d", BornMicrotime.tv_sec, BornMicrotime.tv_usec / 1000);
            s.append(buff);
        }
    };


    struct CSessionKey
    {
        uint16_t             USR; // User Session Reference
        smsc::sms::Address  abonentAddr;

        bool operator ==(const CSessionKey& sk) const
        {
            return ((this->USR == sk.USR) && (this->abonentAddr == sk.abonentAddr));
        }
        //CSessionKey() : USR(-1) {}
        static uint32_t CalcHash(const CSessionKey& key)
        {
            uint32_t retval = (key.abonentAddr.type+key.USR)^key.abonentAddr.plan;
            int i;
            for(i=0;i<key.abonentAddr.length;i++)
            {
                retval=retval*10+(key.abonentAddr.value[i]-'0');
            }
            return retval;
        }
    };

 /*   struct COperationKey
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
    };       */

    class Session;

    class PendingOperation
    {
        friend class Session;
        friend class Comparator;
    public:
        uint8_t type;
        time_t validityTime;
        bool bStartBillingOperation;
        unsigned int billID;
        Logger * logger;

        void rollbackAll(bool timeout = false);

        PendingOperation() : bStartBillingOperation(false), billID(0), logger(0) 
        {
            logger = Logger::getInstance("sess.pop");
        };
    };


    class Operation
    {
        friend class Session;
        friend class Comparator;

        Logger * logger;
        Operation(const Operation& operation);
        bool m_hasBill;
        unsigned int billId;
        int m_receivedParts;
        bool m_receivedAllParts;

        int m_receivedResp;
        bool m_receivedAllResp;

        ICCOperationStatus m_Status;

        std::list<int> DependOperationList;
        std::list<int> DependPendingOperationList;

        Session * m_Owner;
    public:
        uint8_t type;
        uint32_t flags;
        void attachBill(unsigned int BillId);
        void detachBill();
        void rollbackAll();
        bool hasBill() {return m_hasBill;}
        int getBillId() {return billId;}

        void receiveNewPart(int currentIndex,int lastIndex);
        void receiveNewResp(int currentIndex,int lastIndex);

        ICCOperationStatus getStatus();
        void setStatus(ICCOperationStatus status) {m_Status = status;}

        void setFlag(uint32_t f) { flags |= f; };
        void clearFlag(uint32_t f) { flags &= ~f; };
        bool flagSet(uint32_t f) { return flags & f; };

        ~Operation() {}
        Operation(Session * Owner) : 
            m_Owner(Owner),
            logger(0), 
            m_hasBill(false), 
            m_receivedResp(false), 
            m_receivedParts(0), 
            m_receivedAllParts(false), 
            m_receivedAllResp(false),
            m_Status(OPERATION_INITED),
            billId(0)
        {
            logger = Logger::getInstance("sess.op");
        };
    };

    
    typedef IntHash<Operation*> COperationsHash;
    /*
    class COperationsHashPtr : public smsc::core::buffers::RefPtr<COperationsHash,smsc::core::synchronization::Mutex>
    {
    public:
        COperationsHashPtr (COperationsHash * ptr) : smsc::core::buffers::RefPtr(ptr)
        {

        }
        int getRefCount()
        {
            return data->refCount;
        }

    };   */

    //typedef RefPtrWithCounter <Session, smsc::core::synchronization::Mutex> COperationsHashPtr;

    class SessionManagerImpl;

    class Session : public PropertyManager
    {

        friend class Operation;
        friend class SessionManagerImpl;
        friend class Comparator;


        enum ReadOnlyProperties
        {
            PROPERTY_USR = 1,
            PROPERTY_ICC_STATUS = 2
        };

        std::list<SCAGCommand*> cmdQueue;

        SCAGCommand* popCommand()
        { 
            SCAGCommand* c = cmdQueue.front();
            cmdQueue.pop_front();
            return c;
        };
        void pushCommand(SCAGCommand* cmd) { cmdQueue.push_back(cmd); };
        uint32_t commandsEmpty() { return cmdQueue.empty(); };


        Logger * logger;
        std::list<PendingOperation> PendingOperationList;
        std::list<PendingOperation> PrePendingOperationList;

        COperationsHash OperationsHash;
        Operation * m_pCurrentOperation;
        uint64_t currentOperationId;
        unsigned int lastOperationId;

        CSessionKey             m_SessionKey;           //���� ��� SessionManager
        CSessionPrimaryKey      m_SessionPrimaryKey;    //���������� ���� ��� ������ ����������
        bool                    m_bRedirectFlag;

        time_t                  lastAccessTime;
        bool                    bChanged, bDestroy;
        EventMonitor            accessMonitor;
        int                     accessCount;

        Hash<AdapterProperty *> PropertyHash;
        static Hash<int> OperationTypesHash;
        static Hash<int> ReadOnlyPropertiesHash;
        /*
        COperationsHashPtr getOperationsHash()
        {
            COperationsHashPtr opHashPtr(&OperationsHash);
            if (opHashPtr.getRefCount() > 1) abort();

            return opHashPtr;
        }  */
        void ClearOperations();

        int getNewOperationId();
        void DoAddPendingOperation(PendingOperation& pendingOperation);


        void SerializeProperty(SessionBuffer& buff);
        void SerializeOperations(SessionBuffer& buff);
        void SerializePendingOperations(SessionBuffer& buff);

        void DeserializeProperty(SessionBuffer& buff);
        void DeserializeOperations(SessionBuffer& buff);
        void DeserializePendingOperations(SessionBuffer& buff);
        bool m_CanOpenSubmitOperation;
    public:
        void setRedirectFlag() {m_bRedirectFlag = true;}
        bool hasRedirectFlag() {return m_bRedirectFlag;}


        int getPendingAmount() {return PendingOperationList.size();}
        bool getCanOpenSubmitOperation() { return m_CanOpenSubmitOperation;}

        Session(const CSessionKey& key);
        virtual ~Session();

        const CSessionKey& getSessionKey() const  { return m_SessionKey; }
        void setSessionKey(CSessionKey& key) { m_SessionKey = key; }


        inline time_t getLastAccessTime() const { return lastAccessTime; }
        inline bool isChanged() const { return bChanged; }

        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);

        bool hasOperations();
        bool hasPending();
        void closeCurrentOperation();

        void addPendingOperation(PendingOperation pendingOperation);
        void expirePendingOperation();

        void abort();
        Operation * GetCurrentOperation() const;
        Operation * AddNewOperationToHash(SCAGCommand& cmd, int operationType);
        Operation * setCurrentOperation(uint64_t operationId);
        Operation * setCurrentOperationByType(int operationType);
        Operation * setOperationFromPending(SCAGCommand& cmd, int operationType);
        uint64_t getCurrentOperationId();
        
        time_t Session::getWakeUpTime();
        void Serialize(SessionBuffer& buff);
        void Deserialize(SessionBuffer& buff);
        CSessionPrimaryKey& getPrimaryKey() {return m_SessionPrimaryKey;}
        uint16_t getUSR() { return m_SessionKey.USR; }

        static int getOperationType(std::string& str);
        static Hash<int> Session::InitOperationTypesHash();
        static Hash<int> Session::InitReadOnlyPropertiesHash();
        static bool isReadOnlyProperty(const char * name);
    };

    typedef smsc::core::buffers::RefPtr<Session,smsc::core::synchronization::Mutex> SessionPtr;

    class Comparator
    {
        bool compare_properties(SessionPtr s1,SessionPtr s2)
        {
            if (s1->PropertyHash.GetCount() != s2->PropertyHash.GetCount()) return false;

            char * key;
            AdapterProperty * value = 0;

            s1->PropertyHash.First();
            for (Hash <AdapterProperty *>::Iterator it = s1->PropertyHash.getIterator(); it.Next(key, value);)
                if (!s2->PropertyHash.Exists(value->GetName().c_str())) return false;

            return true;
        }

        bool compare_operations(SessionPtr s1,SessionPtr s2)
        {
            Operation * operation;
            int key;

            IntHash <Operation *>::Iterator it = s1->OperationsHash.First();

            if (s1->OperationsHash.Count() != s2->OperationsHash.Count()) return false;

            for (;it.Next(key, operation);)
            {
                if (!s2->OperationsHash.Exist(key)) return false;
                Operation * op2 = s2->OperationsHash.Get(key);

                if (operation->hasBill() != op2->hasBill()) return false;

                if (operation->hasBill())
                    if (operation->billId != op2->billId) return false;


            }
            return true;
        }


        bool compare_pending(SessionPtr s1,SessionPtr s2)
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
        bool compare(SessionPtr s1,SessionPtr s2)
        {
            if (!(compare_properties(s1,s2)&&compare_operations(s1,s2)&&(compare_pending(s1,s2)))) return false;

            if (s1->currentOperationId != s2->currentOperationId) return false;
            if (s1->lastOperationId != s2->lastOperationId) return false;

            bool hasOp1 = (s1->currentOperationId != 0);
            bool hasOp2 = (s2->currentOperationId != 0);

            if (hasOp1 != hasOp2) return false;

            /*if (s1->m_pCurrentOperation != 0)
            {
                if (s1->currentOperationId != s2->currentOperationId) return false;
            } */

            if (s1->m_SessionKey == s2->m_SessionKey); return false;

            if (s1->lastAccessTime != s2->lastAccessTime) return false;

            return true;

        }
    };
  
}}

#endif // SCAG_SESSIONS_SESSION



