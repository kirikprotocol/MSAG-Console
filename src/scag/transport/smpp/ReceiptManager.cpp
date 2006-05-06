
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
//#include "scag/re/CommandBrige.h"

namespace scag { namespace sessions 
{
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::exceptions;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;


    using smsc::logger::Logger;


    class ReceiptManagerImpl : public ReceiptManager
    {
        class XSessionReceiptHashFunc {
        public:
            static uint32_t CalcHash(const CSessionKey& key)
            {
                return XAddrHashFunc::CalcHash(key.abonentAddr);
            }
        };

        typedef XHash<CSessionKey,int,XSessionReceiptHashFunc> CSessionReceiptHash;

        Mutex    inUseMutex;
        CSessionReceiptHash    SessionReceiptHash;
        Logger *        logger;

        std::string storePath;
    public:
        ReceiptManagerImpl() : logger(0) {};
        virtual ~ReceiptManagerImpl();
        void Init(std::string& storePath);

        virtual void waitReceipt(Address& service, CSessionKey& sessionKey);
        virtual void cancelReceipt(Address& service, CSessionKey& sessionKey);
        virtual int updateReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand);
        virtual int registerReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand);

    };


//const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 60;

// ################## Singleton related issues ##################

static bool  bReceiptManagerInited = false;
static Mutex initReceiptManagerLock;

inline unsigned GetLongevity(ReceiptManager*) { return 7; } // ? Move upper ? 
typedef SingletonHolder<ReceiptManagerImpl> SingleRM;


ReceiptManagerImpl::~ReceiptManagerImpl() 
{ 
    smsc_log_debug(logger,"ReceiptManager released");
}


void ReceiptManager::Init(std::string& storePath)
{
    if (!bReceiptManagerInited)
    {
        MutexGuard guard(initReceiptManagerLock);
        if (!bReceiptManagerInited) {
            ReceiptManagerImpl& rm = SingleRM::Instance();
            rm.Init(storePath); 
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

/*

waitReceipt(svc : Address, key : SessionKey)
����������� ������� �������� ������� ��� svc � SessionKey. ���������� ��� ��������� SUBMIT_SM ���� ������� �����
   
cancelReceipt(svc : Address,key : SessionKey)
��������� ������� �������� ������� ��� svc � SessionKey. ���������� ��� ��������� SUBMIT_SM_RESP failed (����� � �� ����-���� �������� SUBMIT � RECEIPT).

updateReceipt(svc : Address, key : SessionKey,smsc_msg_id : string,receipt : SCAGCommand&) : int
��������� ���� �� �������� ������ ��� svc � SessionKey. ���� ��� �� ���������� -1. ����� �� smsc_msg_id ����  ������. ���� ������ ���, �� ��������� ����� � ����� session, ���� receipt ������; ���������� 0. ���� ������ ���� � ���� receipt ��������, �� ������������� sessionKey; ���������� 1 � ���� receipt, ��������� ������� �������� �� svc � SessionKey.
���������� ��� ��������� SUBMIT_SM_RESP ok.

registerReceipt(svc : Address, smsc_msg_id : string, receipt : SCAGCommand, key : SessionKey&) : int
���� �������� �� ����� svc ��� ������, �� ���������� -1. ����� �� smsc_msg_id ����  ������. ���� ������ ���, �� ��������� �����, ���������� ���� receipt, ���� session ������; ���������� 0.
�����, ���� ������ ���� � ���� session  �� ������, �� ���������� 1 � ���� session; ��������� ������� �� svc � SessionKey.


*/



void ReceiptManagerImpl::Init(std::string& _storePath) // possible throws exceptions
{
    this->storePath = _storePath;

    if (!logger)
      logger = Logger::getInstance("scag.ReceiptManager");

    smsc_log_debug(logger,"ReceiptManager::initialized");
}


void ReceiptManagerImpl::waitReceipt(Address& service, CSessionKey& sessionKey)
{
    MutexGuard guard(inUseMutex);
}

void ReceiptManagerImpl::cancelReceipt(Address& service, CSessionKey& sessionKey)
{
    MutexGuard guard(inUseMutex);
    
}

int ReceiptManagerImpl::updateReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand)
{
    MutexGuard guard(inUseMutex);
    return 0;
}

int ReceiptManagerImpl::registerReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand)
{
    MutexGuard guard(inUseMutex);
    return 0;
}


}}


