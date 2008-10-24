
//#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>
//#include <core/synchronization/Event.hpp>
//#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/XHash.hpp>

#include <scag/util/singleton/Singleton.h>

#include <unistd.h>
#include <time.h>
#include <logger/Logger.h>

#include "ReceiptManager.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/sms/HashUtil.h"
#include "scag/transport/smpp/SmppCommand.h"

namespace scag { namespace sessions 
{
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::exceptions;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;

    using namespace scag::transport::smpp;

    using smsc::logger::Logger;


    class ReceiptManagerImpl : public ReceiptManager
    {

        class Receipt
        {
            SCAGCommand * command;
            CSessionKey * sessionKey;

            Receipt (const Receipt& sm);
            Receipt& operator=(const Receipt& sm);
        public:
            Receipt() : command(0), sessionKey(0) {}
            ~Receipt()
            {
                if (command) delete command;
                if (sessionKey) delete sessionKey;
            }

            void setSessionKey(const CSessionKey newSessionKey)
            {
                if (!sessionKey) sessionKey = new CSessionKey();

                (*sessionKey) = newSessionKey;
            }

            void setSCAGCommand(SmppCommand& smppCommand)
            {
                if (!command) command = new SmppCommand();

                //TODO: ensure that copy is available
                (*command) = smppCommand;

            }

            SCAGCommand * getCommand()
            {
                return command;
            }

            CSessionKey * getSessionKey()
            {
                return sessionKey;
            }
        };

     /*
        struct ReceiptInfo
        {
            int waitReceipts;
            CSessionKey sessionKey;
            ReceiptInfo() : waitReceipts(0) {}
        };
        */
        struct ReceiptKey
        {
            Address service;
            CSessionKey sessionKey;

            ReceiptKey(const Address& _service, const CSessionKey& _sessionKey) :
                service(_service), sessionKey(_sessionKey) {}

            ReceiptKey()
            {
            }

            bool operator == (const ReceiptKey& key)
            {
                return ((key.service == service)&&(key.sessionKey == sessionKey));
            }
        };

        class XSessionReceiptHashFunc {
        public:
            static uint32_t CalcHash(const ReceiptKey& key)
            {
                return XAddrHashFunc::CalcHash(key.service);
            }
        };    

        typedef XHash<ReceiptKey, int, XSessionReceiptHashFunc> CSessionReceiptHash;

        Mutex    inUseMutex;
        CSessionReceiptHash SessionReceiptHash;
        Hash<Receipt *> Receipts;

        Logger * logger;

    public:
        ReceiptManagerImpl() : logger(0) {};
        virtual ~ReceiptManagerImpl();
        void Init();

        virtual void waitReceipt(Address& service, CSessionKey& sessionKey);
        virtual void cancelReceipt(Address& service, CSessionKey& sessionKey);
        virtual int updateReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand);
        virtual int registerReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand);

    };



// ################## Singleton related issues ##################

static bool  bReceiptManagerInited = false;
static Mutex initReceiptManagerLock;

inline unsigned GetLongevity(ReceiptManager*) { return 249; } // ? Move upper ?
typedef SingletonHolder<ReceiptManagerImpl> SingleRM;


ReceiptManagerImpl::~ReceiptManagerImpl() 
{ 

    char * key;
    Receipt * receipt = 0;

    Receipts.First();

    for (Hash <Receipt *>::Iterator it = Receipts.getIterator(); it.Next(key, receipt);)
    {
        delete receipt;
    }

    smsc_log_debug(logger,"ReceiptManager released");
}


void ReceiptManager::Init()
{
    if (!bReceiptManagerInited)
    {
        MutexGuard guard(initReceiptManagerLock);
        if (!bReceiptManagerInited) {
            ReceiptManagerImpl& rm = SingleRM::Instance();
            rm.Init(); 
            bReceiptManagerInited = true;
        }
    }
}
ReceiptManager& ReceiptManager::Instance()
{
    if (!bReceiptManagerInited) 
    {
        MutexGuard guard(initReceiptManagerLock);
        if (!bReceiptManagerInited) 
            throw std::runtime_error("ReceiptManager not inited!");
    }
    return SingleRM::Instance();
}

// ################ TODO: Actual ReceiptManager Implementation follows ################ 



void ReceiptManagerImpl::Init() // possible throws exceptions
{
    if (!logger)
      logger = Logger::getInstance("scag.ReceiptManager");

    smsc_log_debug(logger,"ReceiptManager::initialized");
}

//Увеличивает счётчик ожиданий отчётов для svc и SessionKey. Вызывается при обработке SUBMIT_SM если заказан отчёт

void ReceiptManagerImpl::waitReceipt(Address& service, CSessionKey& sessionKey)
{
    MutexGuard guard(inUseMutex);

    ReceiptKey key(service, sessionKey);

    int * pCount = SessionReceiptHash.GetPtr(key);
  
    if (pCount) 
    {
        (*pCount)  = (*pCount) + 1;
    }
    else
    {
        SessionReceiptHash.Insert(key,1);
    }
   
}

//Уменьшает счётчик ожиданий отчётов для svc и SessionKey. Вызывается при обработке SUBMIT_SM_RESP failed 
//(также и по тайм-ауту операций SUBMIT и RECEIPT).
void ReceiptManagerImpl::cancelReceipt(Address& service, CSessionKey& sessionKey)
{
    MutexGuard guard(inUseMutex);

    ReceiptKey key(service, sessionKey);

    int * pCount = SessionReceiptHash.GetPtr(key);
    
    if (pCount) 
    {
        (*pCount)  = (*pCount) - 1;
    }
    
}

//Проверяет есть ли ожидание отчёта для svc и SessionKey. Если нет то возвращает -1. 
//Иначе по smsc_msg_id ищет  запись. Если записи нет, то добавляет новую с полем session, 
//поле receipt пустое; возвращает 0. Если запись есть и поле receipt непустое, то устанавливает sessionKey; 
//возвращает 1 и поле receipt, уменьшает счётчик ожиданий по svc и SessionKey.
//Вызывается при обработке SUBMIT_SM_RESP ok.

//А если запись есть и поле receipt пустое? Тогда что возвращяем?
int ReceiptManagerImpl::updateReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand)
{
    MutexGuard guard(inUseMutex);

    ReceiptKey key(service, sessionKey);

    int * pCount = SessionReceiptHash.GetPtr(key);

    if (!pCount) return -1;

    Receipt ** pReceipt = Receipts.GetPtr(smsc_msg_id.c_str());
    
    if (!pReceipt) 
    {
        Receipt * receipt = new Receipt();

        receipt->setSessionKey(sessionKey);
        Receipts.Insert(smsc_msg_id.c_str(), receipt);
        return 0;
    }

    if ((*pReceipt)->getCommand())
    {
        (*pReceipt)->setSessionKey(sessionKey);
        //TODO: ensure that copy is available
        receiptCommand = *((*pReceipt)->getCommand());
        return 1;
    }
    
    throw SCAGException("ReceiptManager: Cannot update receipt");
    
}

//Если ожиданий по этому svc нет вообще, то возвращает -1. Иначе по smsc_msg_id ищет  запись. 
//Если записи нет, то добавляет новую, выставляет поле receipt, поле session пустое; возвращает 0.
//Иначе, если запись есть и поле session  не пустое, то возвращает 1 и поле session; 
//уменьшает счётчик по svc и SessionKey.

int ReceiptManagerImpl::registerReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand)
{
    MutexGuard guard(inUseMutex);

    ReceiptKey key(service, sessionKey);

    int * pMessageId = SessionReceiptHash.GetPtr(key);
    if (!pMessageId) return -1;

    (*pMessageId) = (* pMessageId) - 1;

    //SessionReceiptHash
  /*  SessionReceiptHash.First();
    CSessionReceiptHash::Iterator it = SessionReceiptHash.getIterator();

    ReceiptKey key; 
    int value;

    bool findService = false;

    while (it.Next(key, value))
    {
        if (key.service == service) 
        {
            findService = true;
            break;
        }
    }

    if (!findService) return -1;*/

    Receipt ** pReceiptPtr = Receipts.GetPtr(smsc_msg_id.c_str());

    if (!pReceiptPtr) 
    {
        Receipt * receipt = new Receipt();

        SmppCommand * smppReceiptCommand = dynamic_cast<SmppCommand *>(&receiptCommand);

        if (!smppReceiptCommand) throw SCAGException("ReceiptManager: Cannot register receipt - command is not smpp-type");

        receipt->setSCAGCommand(*smppReceiptCommand);

        Receipts.Insert(smsc_msg_id.c_str(), receipt);
        return 0;
    }

    CSessionKey * pSessionKey = (*pReceiptPtr)->getSessionKey();
    if (pSessionKey) 
    {
        sessionKey = *pSessionKey;

        return 1;
    }

    throw SCAGException("ReceiptManager: Cannot register receipt");
}


}}


