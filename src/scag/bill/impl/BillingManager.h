#ifndef _SCAG_BILL_IMPL_BILLINGMANAGER_H
#define _SCAG_BILL_IMPL_BILLINGMANAGER_H

#include "scag/bill/base/BillingManager.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include <core/buffers/IntHash.hpp>
#include "core/network/Socket.hpp"
#include "scag/config/base/ConfigListener2.h"
#include "scag/config/bill/BillingManagerConfig.h"
#include "scag/stat/base/Statistics2.h"
#include "Infrastructure.h"

#ifdef MSAG_INMAN_BILL
#include "inman/interaction/connect.hpp"
#include "inman/interaction/msgbill/MsgBilling.hpp"
#endif


namespace scag2 {
namespace bill {

using namespace smsc::core::threads;
using smsc::core::network::Socket;
// using namespace scag::exceptions;
using smsc::logger::Logger;

class BillingManagerImpl :
public BillingManager,
public Thread,
public config::ConfigListener
#ifdef MSAG_INMAN_BILL
    , public SMSCBillingHandlerITF, public ConnectListenerITF
#endif
{
    struct BillTransaction
    {
        TransactionStatus status;
        TariffRec tariffRec;
        #ifdef MSAG_INMAN_BILL
        SPckChargeSms ChargeOperation;
        #endif
        unsigned int billId;
        stat::SaccBillingInfoEvent billEvent;
        BillingInfoStruct billingInfoStruct;
        BillTransaction() : status(TRANSACTION_NOT_STARTED) {}
    };
    #ifdef MSAG_INMAN_BILL
    struct SendTransaction
    {
        TransactionStatus status;
        Event responseEvent;
        LongCallContext* lcmCtx;
        time_t startTime;
        BillTransaction* billTransaction;
        SendTransaction() : status(TRANSACTION_WAIT_ANSWER), lcmCtx(NULL), startTime(time(NULL)), billTransaction(NULL) {}
    };
    IntHash <SendTransaction *> SendTransactionHash;
    #endif

    Logger *logger;

    IntHash <BillTransaction *> BillTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock, sendLock;
    Event exitEvent;

    bool m_bStarted;

    unsigned int m_lastBillId;
    int m_ReconnectTimeout, m_Timeout;

    bool m_Connected;
    int m_Port;
    std::string m_Host;

    bool isStarted() { return m_bStarted; };

    int max_t, min_t, billcount, start_t;
    InfrastructureImpl infrastruct;

    #ifdef MSAG_INMAN_BILL
    Socket * socket;
    Connect * pipe;
    virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd);
    virtual void onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc);
    void sendCommand(INPPacketAC& op) { pipe->sendPck(&op); }
    TransactionStatus sendCommandAndWaitAnswer(SPckChargeSms& op);

    void fillChargeSms(smsc::inman::interaction::ChargeSms& op, BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec);
    virtual void onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr);
    bool Reconnect();
    void InitConnection(std::string& host, int port)
    {
        m_Host = host;
        m_Port = port;
    }
    void insertSendTransaction(int dlgId, SendTransaction* st);
    void deleteSendTransaction(int dlgId);

    void sendCommandAsync(BillTransaction *bt, LongCallContext* lcmCtx);
    void processAsyncResult(BillingManagerImpl::SendTransaction* pst);
    #endif /* MSAG_INMAN_BILL */

    void ProcessResult(const char* eventName, BillingTransactionEvent billingTransactionEvent, BillTransaction * billTransaction);

    void makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo,
                       stat::SaccBillingInfoEvent* ev);

    uint32_t genBillId();
    BillTransaction* getBillTransaction(uint32_t billId);
    void putBillTransaction(uint32_t billId, BillTransaction* p);
    void ClearTransactions()
    {
        int key;

#ifdef MSAG_INMAN_BILL
        {
            MutexGuard mg1(sendLock);
            SendTransaction *st;
            for(IntHash <SendTransaction *>::Iterator it = SendTransactionHash.First(); it.Next(key, st);)
            {
                if(st->lcmCtx)
                {
                    st->lcmCtx->initiator->continueExecution(st->lcmCtx, true);
                    delete st;
                }
                else
                    st->responseEvent.Signal();
            }
        }
#endif

        BillTransaction *value;
        MutexGuard mg(inUseLock);
        for (IntHash <BillTransaction *>::Iterator it = BillTransactionHash.First(); it.Next(key, value);)
            delete value;

        BillTransactionHash.Empty();
    }
    void logEvent(const char *type, bool success, BillingInfoStruct& b, int billID);

public:
    void init( config::BillingManagerConfig& cfg );

    virtual int Execute();
    virtual void Start();
    virtual void Stop();        

    virtual unsigned int Open( BillingInfoStruct& billingInfoStruct,
                               TariffRec& tariffRec,
                               lcm::LongCallContext* lcmCtx = NULL);
    virtual void Commit( int billId, lcm::LongCallContext* lcmCtx = NULL);
    virtual void Rollback( int billId, bool timeout, lcm::LongCallContext* lcmCtx = NULL );
    virtual void Info( int billId, BillingInfoStruct& bis, TariffRec& tariffRec);

    virtual Infrastructure& getInfrastructure() { return infrastruct; };

    void configChanged();

    BillingManagerImpl() :
    ConfigListener(config::BILLMAN_CFG),
    logger(Logger::getInstance("bill.man")),
    m_bStarted(false),
    m_lastBillId(0)
#ifdef MSAG_INMAN_BILL
        , socket(0), pipe(0)
#endif
    {
     max_t = 0, min_t = 1000000000, billcount = 0, start_t = time(NULL);
        #ifdef MSAG_INMAN_BILL
        socket = new Socket();
        INPSerializer::getInstance()->registerCmdSet(INPCSBilling::getInstance());
        pipe = new Connect(socket, INPSerializer::getInstance(), logger);
        pipe->addListener(this);
        #endif
    }

    ~BillingManagerImpl()
    {
        #ifdef MSAG_INMAN_BILL
        if(pipe) delete pipe;
        #endif
    }

};

} // namespace bill
} // namespace scag2

#endif /* ! _SCAG_BILL_IMPL_BILLINGMANAGER_H */
