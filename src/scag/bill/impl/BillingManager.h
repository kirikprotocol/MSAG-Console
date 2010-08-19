#ifndef _SCAG_BILL_IMPL_BILLINGMANAGER_H
#define _SCAG_BILL_IMPL_BILLINGMANAGER_H

#include "scag/bill/base/BillingManager.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/threads/Thread.hpp>
#include "core/buffers/IntHash.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/network/Socket.hpp"
#include "scag/config/base/ConfigListener2.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/stat/base/Statistics2.h"
#include "Infrastructure.h"
#include "scag/bill/ewallet/Client.h"

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

#ifdef MSAG_INMAN_BILL
using smsc::core::network::Socket;
#endif

class BillingManagerImplTester;

class BillingManagerImpl :
public BillingManager,
public Thread,
public config::ConfigListener,
public EwalletCallParams::TransactionRegistrator
#ifdef MSAG_INMAN_BILL
    , public smsc::inman::interaction::SMSCBillingHandlerITF
    , public smsc::inman::interaction::ConnectListenerITF
#endif
{
    friend class BillingManagerImplTester;

    struct BillTransaction
    {
        TransactionStatus status;
        TariffRec tariffRec;
        #ifdef MSAG_INMAN_BILL
        smsc::inman::interaction::SPckChargeSms ChargeOperation;
        #endif
        billid_type billId;
        stat::SaccBillingInfoEvent billEvent;
        BillingInfoStruct billingInfoStruct;

        uint32_t ewalletTransId; // used to identify an Ewallet transaction
        BillTransaction() : status(TRANSACTION_NOT_STARTED), ewalletTransId(0) {}
    };

    #ifdef MSAG_INMAN_BILL
    struct SendTransaction
    {
        TransactionStatus status;
        Event responseEvent;
        lcm::LongCallContext* lcmCtx;
        time_t startTime;
        BillTransaction* billTransaction;
        SendTransaction() : status(TRANSACTION_WAIT_ANSWER), lcmCtx(NULL), startTime(time(NULL)), billTransaction(NULL) {}
    };
    // by dialog number
    IntHash<SendTransaction *> SendTransactionHash;
    #endif

    Logger *logger;

    // by trans number
    IntHash64<BillTransaction *> BillTransactionHash;

    Event connectEvent;
    Mutex stopLock;
    Mutex inUseLock, sendLock;
    Event exitEvent;

    bool m_bStarted;

    billid_type m_lastBillId;
    int lastDlgId;

    int m_ReconnectTimeout, m_Timeout;

    bool m_Connected;
    int m_Port;
    std::string m_Host;

    bool isStarted() { return m_bStarted; };

    int max_t, min_t, billcount;
    time_t start_t;
    InfrastructureImpl infrastruct;
    std::auto_ptr<ewallet::Client> ewalletClient_;


    #ifdef MSAG_INMAN_BILL
    Socket * socket;
    smsc::inman::interaction::Connect * pipe;
    virtual void onPacketReceived( smsc::inman::interaction::Connect* conn,
                                   std::auto_ptr<SerializablePacketAC>& recv_cmd );
    virtual void onConnectError( smsc::inman::interaction::Connect* conn,
                                 std::auto_ptr<CustomException>& p_exc);
    void sendCommand( smsc::inman::interaction::INPPacketAC& op ) {
        pipe->sendPck(&op); 
    }
    TransactionStatus sendCommandAndWaitAnswer( smsc::inman::interaction::SPckChargeSms& op );

    void fillChargeSms( smsc::inman::interaction::ChargeSms& op,
                        BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec );
    virtual void onChargeSmsResult( smsc::inman::interaction::ChargeSmsResult* result,
                                    smsc::inman::interaction::CsBillingHdr_dlg * hdr );
    bool Reconnect();
    void InitConnection(std::string& host, int port)
    {
        m_Host = host;
        m_Port = port;
    }
    void insertSendTransaction(int dlgId, SendTransaction* st);
    void deleteSendTransaction(int dlgId);

    void sendCommandAsync( BillTransaction *bt, lcm::LongCallContext* lcmCtx );
    void processAsyncResult(BillingManagerImpl::SendTransaction* pst);

    #endif /* MSAG_INMAN_BILL */

    void ProcessResult(const char* eventName, BillingTransactionEvent billingTransactionEvent, BillTransaction * billTransaction);

    void makeBillEvent(BillingTransactionEvent billCommand, BillingCommandStatus commandStatus, TariffRec& tariffRec, BillingInfoStruct& billingInfo,
                       stat::SaccBillingInfoEvent* ev);

    billid_type genBillId();
    BillTransaction* getBillTransaction(billid_type billId);
    void putBillTransaction(billid_type billId, BillTransaction* p);
    void ClearTransactions()
    {

#ifdef MSAG_INMAN_BILL
        {
            MutexGuard mg1(sendLock);
            int key;
            SendTransaction *st;
            for(IntHash<SendTransaction *>::Iterator it = SendTransactionHash.First(); it.Next(key, st);)
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
        billid_type key;
        for (IntHash64<BillTransaction *>::Iterator it = BillTransactionHash.First(); it.Next(key, value);)
            delete value;

        BillTransactionHash.Empty();
    }
    void logEvent(const char *type, bool success, BillingInfoStruct& b, billid_type billID);

    int makeInmanId( billid_type billid );
    virtual void processAsyncResult( EwalletCallParams& params );

public:
    void init( config::ConfigManager& cfg );

    virtual int Execute();
    virtual void Start();
    virtual void Stop();        

    virtual billid_type Open( BillOpenCallParams& openCallParams,
                               lcm::LongCallContext* lcmCtx = NULL);
    virtual void Commit( billid_type billId, lcm::LongCallContext* lcmCtx = NULL);
    virtual void Rollback( billid_type billId, bool timeout, 
                           lcm::LongCallContext* lcmCtx = NULL );
    virtual void CommitTransit( BillCloseCallParams& closeCallParams,
                                lcm::LongCallContext* lcmCtx = NULL );
    virtual void RollbackTransit( BillCloseCallParams& closeCallParams,
                                  lcm::LongCallContext* lcmCtx = NULL );
    virtual void Check( BillCheckCallParams& checkCallParams,
                        lcm::LongCallContext* lcmCtx );
    virtual void Info( billid_type billId, BillingInfoStruct& bis, TariffRec& tariffRec);
    virtual void Info( EwalletInfoCallParams& infoParams,
                       lcm::LongCallContext* lcmCtx );
    virtual void Transfer( BillTransferCallParams& callParams,
                           lcm::LongCallContext* lcmCtx );

    virtual Infrastructure& getInfrastructure() { return infrastruct; };

    void configChanged();

    BillingManagerImpl() :
    ConfigListener(config::BILLMAN_CFG),
    logger(0),
    m_bStarted(false),
    m_lastBillId(util::currentTimeMillis()),
    lastDlgId(0)
#ifdef MSAG_INMAN_BILL
        , socket(0), pipe(0)
#endif
    {
        logger = Logger::getInstance("bill.man");
        max_t = 0, min_t = 1000000000, billcount = 0, start_t = time(NULL);
        #ifdef MSAG_INMAN_BILL
        socket = new Socket();
        smsc::inman::interaction::INPSerializer::getInstance()
            ->registerCmdSet(smsc::inman::interaction::INPCSBilling::getInstance());
        pipe = new smsc::inman::interaction::Connect
            ( socket, smsc::inman::interaction::INPSerializer::getInstance(), logger);
        pipe->addListener(this);
        #endif
    }

    ~BillingManagerImpl()
    {
        #ifdef MSAG_INMAN_BILL
        if(pipe) delete pipe;
        #endif
    }

private:
    void Start( int ); // to make compiler happy

};

} // namespace bill
} // namespace scag2

#endif /* ! _SCAG_BILL_IMPL_BILLINGMANAGER_H */
