/* "$Id$" */
#include "BillingManager.h"

#include "scag/exc/SCAGExceptions.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/util/lltostr.h"

#ifdef MSAG_INMAN_BILL
#include "inman/services/smbill/SmBillDefs.hpp"
#endif

namespace scag2 {
namespace bill {

#ifdef MSAG_INMAN_BILL
using namespace smsc::inman::interaction;
#endif

using namespace scag::exceptions;


void BillingManagerImpl::init( config::BillingManagerConfig& cfg )
{
    MutexGuard guard(inUseLock);
    smsc_log_info(logger, "BillingManager start initing...");

    if (cfg.MaxThreads < 1) throw SCAGException("BillingManager: cant start service with %d allowed threads", cfg.MaxThreads);

    m_ReconnectTimeout = cfg.BillingReconnectTimeOut;
    m_Timeout = cfg.BillingTimeOut;

    #ifdef MSAG_INMAN_BILL
    InitConnection(cfg.BillingHost, cfg.BillingPort);
    smsc_log_info(logger,"BillingManager connecting to host '%s', port %d", cfg.BillingHost.c_str(),cfg.BillingPort);
    if(socket->Init(m_Host.c_str(), m_Port, 1000))
        throw SCAGException("BillingManager: cant init socket host=%s, port=%d", m_Host.c_str(), m_Port);
    m_Connected = Reconnect();
    #else
    smsc_log_info(logger,"BillingManager skip connection (fake connection initialized)");
    m_Connected = true;
    #endif

    infrastruct.init(cfg.cfg_dir);
    smsc_log_info(logger,"BillingManager inited...");
}

void BillingManagerImpl::configChanged()
{
    MutexGuard guard(inUseLock);
    Stop();
    init(config::ConfigManager::Instance().getBillManConfig());
    Start();
}

void BillingManagerImpl::ProcessResult(const char *eventName, BillingTransactionEvent event, BillTransaction *b)
{
    BillingCommandStatus i;
    const char *p;
    switch (b->status)
    {
        case TRANSACTION_INVALID:
            i = INVALID_TRANSACTION;
            p = "invalid(charging is impossible?)";
            break;
        case TRANSACTION_NOT_STARTED:
            i = REJECTED_BY_SERVER;
            p = "denied";
            break;
        case TRANSACTION_WAIT_ANSWER:
            i = SERVER_NOT_RESPONSE;
            p = "time out";
            break;
        default:
            i = COMMAND_SUCCESSFULL;
    }
    stat::SaccBillingInfoEvent* billEvent = new stat::SaccBillingInfoEvent();
    makeBillEvent(event, i, b->tariffRec, b->billingInfoStruct, billEvent);
    stat::Statistics::Instance().registerSaccEvent(billEvent);
    logEvent(eventName, i == COMMAND_SUCCESSFULL, b->billingInfoStruct, b->billId);
    if(i != COMMAND_SUCCESSFULL)
        throw SCAGException("Transaction billId=%d %s", b->billId, p);
}
#ifdef MSAG_INMAN_BILL
void BillingManagerImpl::processAsyncResult(BillingManagerImpl::SendTransaction* pst)
{
    auto_ptr<SendTransaction> st(pst);
    auto_ptr<BillTransaction> bt(st->billTransaction);

    smsc_log_debug(logger, "ProcessAsyncResult billId=%d", bt->billId);

    SendTransactionHash.Delete(bt->billId);

    lcm::LongCallContext* lcmCtx = st->lcmCtx;

    BillingCommandStatus i;
    const char *p;

    bt->status = st->status;

    if( st->status == TRANSACTION_VALID )
    {
        SPckDeliverySmsResult opRes;
        opRes.Hdr().dlgId = bt->billId;
        if( lcmCtx->callCommandId == lcm::BILL_OPEN )
        {
            BillOpenCallParams* bp = (BillOpenCallParams*)lcmCtx->getParams();
            bp->BillId = bt->billId;

            opRes.Cmd().setResultValue(1);
            opRes.Cmd().setFinal(false); // To skip CDR creation
        }
        else if(lcmCtx->callCommandId == lcm::BILL_ROLLBACK)
            opRes.Cmd().setResultValue(2);
        sendCommand(opRes);

        i = COMMAND_SUCCESSFULL;
    }
    else
    {
        switch (st->status)
        {
            case TRANSACTION_INVALID:
                i = INVALID_TRANSACTION;
                p = "invalid(charging is impossible?)";
                break;
            case TRANSACTION_NOT_STARTED:
                i = REJECTED_BY_SERVER;
                p = "denied";
                break;
            case TRANSACTION_WAIT_ANSWER:
            default:
                i = SERVER_NOT_RESPONSE;
                p = "time out";
                break;
        }
    }

    const char *eventName;
    BillingTransactionEvent event;
    switch(lcmCtx->callCommandId)
    {
    case lcm::BILL_OPEN: event = TRANSACTION_OPEN; eventName = "open"; break;
    case lcm::BILL_COMMIT: event = TRANSACTION_COMMITED; eventName = "commit"; break;
    case lcm::BILL_ROLLBACK: event = TRANSACTION_CALL_ROLLBACK; eventName = "rollback"; break;
    }

    stat::SaccBillingInfoEvent* billEvent = new stat::SaccBillingInfoEvent();
    makeBillEvent( event, i, bt->tariffRec, bt->billingInfoStruct, billEvent );
    stat::Statistics::Instance().registerSaccEvent(billEvent);
    logEvent(eventName, i == COMMAND_SUCCESSFULL, bt->billingInfoStruct, bt->billId);
    if(i != COMMAND_SUCCESSFULL)
    {
        char buf[20];
        buf[19]=0;
        lcm::LongCallParams* lp = lcmCtx->getParams();
        lp->exception = "Transaction billId=";
        lp->exception += lltostr(bt->billId, buf + 19);
        lp->exception += p;
    }
    else if ( lcmCtx->callCommandId == lcm::BILL_OPEN) {
        const unsigned int billid = bt->billId;
        putBillTransaction( billid, bt.release());
    }
    
    lcmCtx->initiator->continueExecution(lcmCtx, false);
}
#endif

void BillingManagerImpl::logEvent(const char *tp, bool success, BillingInfoStruct& b, int billID)
{
    smsc_log_info(logger, "bill %s: %s billId=%d, abonent=%s, opId=%d, sId=%d, providerId=%d",
        tp, success ? "success" : "failed", billID, b.AbonentNumber.c_str(), b.operatorId, b.serviceId, b.providerId);
}

BillingManagerImpl::BillTransaction* BillingManagerImpl::getBillTransaction(uint32_t billId)
{
    MutexGuard mg(inUseLock);
    BillTransaction** pp = BillTransactionHash.GetPtr(billId);
    if (!pp || !*pp) throw SCAGException("Cannot find transaction for billId=%d", billId);
    BillTransactionHash.Delete(billId);
    return *pp;
}

void BillingManagerImpl::putBillTransaction(uint32_t billId, BillTransaction* p)
{
    MutexGuard mg(inUseLock);
    BillTransactionHash.Insert(billId, p);
}

uint32_t BillingManagerImpl::genBillId()
{
    MutexGuard mg(inUseLock);
    return ++m_lastBillId;
}

unsigned int BillingManagerImpl::Open( BillingInfoStruct& billingInfoStruct,
                                       TariffRec& tariffRec,
                                       lcm::LongCallContext* lcmCtx)
{
    uint32_t billId = genBillId();

    smsc_log_debug(logger, "Opening billId=%d...", billId);

    auto_ptr<BillTransaction> p(new BillTransaction());

    p->tariffRec = tariffRec;
    p->billingInfoStruct = billingInfoStruct;
    p->billId = billId;
//    makeBillEvent(TRANSACTION_OPEN, COMMAND_SUCCESSFULL, tariffRec, billingInfoStruct, p->billEvent);

    #ifdef MSAG_INMAN_BILL
    if(tariffRec.billType == bill::infrastruct::INMAN || tariffRec.billType == bill::infrastruct::INMANSYNC)
    {
        fillChargeSms(p->ChargeOperation.Cmd(), billingInfoStruct, tariffRec);
        p->ChargeOperation.Hdr().dlgId = billId;

        if(lcmCtx)
        {
            smsc_log_debug( logger, "going to send command async" );
            sendCommandAsync(p.release(), lcmCtx);
            return 0;
        }

        smsc_log_debug(logger, "Send sync inman command billid=%d", billId);
        p->status = sendCommandAndWaitAnswer(p->ChargeOperation);
        if(p->status == TRANSACTION_VALID)
        {
            SPckDeliverySmsResult opRes;
            opRes.Hdr().dlgId = billId;
            opRes.Cmd().setResultValue(1);
            opRes.Cmd().setFinal(false); // To skip CDR creation
            sendCommand(opRes);
        }
    }
    else
    #endif
        p->status = TRANSACTION_VALID;

    ProcessResult("open", TRANSACTION_OPEN, p.get());

    putBillTransaction(billId, p.release());

    return billId;
}

void BillingManagerImpl::Commit(int billId, lcm::LongCallContext* lcmCtx)
{
    smsc_log_debug(logger, "Commiting billId=%d...", billId);

    auto_ptr<BillTransaction> p(getBillTransaction(billId));

    #ifdef MSAG_INMAN_BILL
    if(p->tariffRec.billType == bill::infrastruct::INMAN || p->tariffRec.billType == bill::infrastruct::INMANSYNC)
    {
        if(lcmCtx)
        {
            sendCommandAsync(p.release(), lcmCtx);
            return;
        }

        p->status = sendCommandAndWaitAnswer(p->ChargeOperation);
        if(p->status == TRANSACTION_VALID)
        {
            SPckDeliverySmsResult op;
            op.Hdr().dlgId = billId;
            sendCommand(op);
        }
    }
    else
    #endif
        p->status = TRANSACTION_VALID;

    ProcessResult("commit", TRANSACTION_COMMITED, p.get());
}

void BillingManagerImpl::Rollback(int billId, bool timeout, lcm::LongCallContext* lcmCtx)
{
    smsc_log_debug(logger, "Rolling back billId=%d...", billId);

    auto_ptr<BillTransaction> p(getBillTransaction(billId));

/*    #ifdef MSAG_INMAN_BILL
    if (p->status == TRANSACTION_VALID && p->tariffRec.billType == bill::infrastruct::INMAN)
    {
        SPckDeliverySmsResult op;
        op.Hdr().dlgId = billId;
        op.Cmd().setResultValue(2);
        sendCommand(op);
    }
    #endif
*/
    ProcessResult(timeout ? "rollback(timeout)" : "rollback", 
            timeout ? TRANSACTION_TIME_OUT : TRANSACTION_CALL_ROLLBACK, p.get());
}

void BillingManagerImpl::Info(int billId, BillingInfoStruct& bis, TariffRec& tariffRec)
{
    MutexGuard mg(inUseLock);
    BillTransaction** p = BillTransactionHash.GetPtr(billId);
    if (!p || !*p) throw SCAGException("Cannot find transaction for billId=%d", billId);
    bis = (*p)->billingInfoStruct;
    tariffRec = (*p)->tariffRec;
}

void BillingManagerImpl::makeBillEvent( BillingTransactionEvent billCommand,
                                        BillingCommandStatus commandStatus,
                                        TariffRec& tariffRec,
                                        BillingInfoStruct& billingInfo,
                                        stat::SaccBillingInfoEvent* ev)
{
    ev->Header.cCommandId = billCommand;
    ev->Header.cProtocolId = billingInfo.protocol;
    ev->Header.iServiceId = billingInfo.serviceId;
    ev->Header.iServiceProviderId = billingInfo.providerId;

    timeval tv;
    gettimeofday(&tv,0);

    ev->Header.lDateTime = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec / 1000);
    ev->Header.pAbonentNumber = billingInfo.AbonentNumber;
    ev->Header.sCommandStatus = commandStatus;
    ev->Header.iOperatorId = billingInfo.operatorId;

    ev->iPriceCatId = tariffRec.CategoryId;
    ev->fBillingSumm = tariffRec.Price;
    ev->iMediaResourceType = tariffRec.MediaTypeId;
    ev->pBillingCurrency = tariffRec.Currency;

    char buff[70];
    sprintf(buff,"%s/%ld%d",billingInfo.AbonentNumber.c_str(), billingInfo.SessionBornMicrotime.tv_sec, int(billingInfo.SessionBornMicrotime.tv_usec / 1000));
    ev->Header.pSessionKey.append(buff);
}

#ifdef MSAG_INMAN_BILL
void BillingManagerImpl::fillChargeSms(smsc::inman::interaction::ChargeSms& op, BillingInfoStruct& billingInfoStruct, TariffRec& tariffRec)
{
    op.setDestinationSubscriberNumber(tariffRec.ServiceNumber);
    op.setCallingPartyNumber(billingInfoStruct.AbonentNumber);
    op.setServiceId(billingInfoStruct.serviceId);
    op.setUserMsgRef(billingInfoStruct.msgRef);
    op.setSmsXSrvs(SMSX_INCHARGE_SRV);

    smsc_log_debug(logger, "***** SN=%s, CPN=%s, SID=%d", tariffRec.ServiceNumber.c_str(), billingInfoStruct.AbonentNumber.c_str(), billingInfoStruct.serviceId);
}

void BillingManagerImpl::onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr)
{
    if (!result) return;

    MutexGuard guard(sendLock);

    SendTransaction **p = SendTransactionHash.GetPtr(hdr->dlgId);
    if(!p)
    {
        smsc_log_error(logger, "Cannot find transaction for billId=%d", hdr->dlgId);
        //TODO: do what we must to do
        return;
    }

    (*p)->status = result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE ? TRANSACTION_VALID : TRANSACTION_INVALID;

    if((*p)->lcmCtx)
        processAsyncResult(*p);
    else
        (*p)->responseEvent.Signal();
}

bool BillingManagerImpl::Reconnect()
{
    if(!socket->Connect()) 
    {
//        socket->SetNoDelay(true);
        pipe->Reset();
        smsc_log_warn(logger, "Connected socket to BillingServer on host '%s', port '%d'", m_Host.c_str(), m_Port);
        return true;
    }
    smsc_log_warn(logger, "Can't connect socket to BillingServer on host '%s', port '%d': error %s (%d)", m_Host.c_str(), m_Port, strerror(errno), errno);
    return false;
}

void BillingManagerImpl::insertSendTransaction(int dlgId, SendTransaction* st)
{
    MutexGuard mg(sendLock);
    SendTransactionHash.Insert(dlgId, st);
}

void BillingManagerImpl::deleteSendTransaction(int dlgId)
{
    MutexGuard mg(sendLock);
    SendTransactionHash.Delete(dlgId);
}

void BillingManagerImpl::sendCommandAsync(BillTransaction *bt, lcm::LongCallContext* lcmCtx)
{
    SendTransaction* s = new SendTransaction();
    auto_ptr<SendTransaction> st(s);

    st->lcmCtx = lcmCtx;
    st->billTransaction = bt;
    insertSendTransaction(bt->billId, st.release());
    if(pipe->sendPck(&bt->ChargeOperation) <= 0)
    {
        MutexGuard mg(sendLock);
        processAsyncResult(s);
    }
}

TransactionStatus BillingManagerImpl::sendCommandAndWaitAnswer(SPckChargeSms& op)
{
    if(!m_Connected)
        return TRANSACTION_WAIT_ANSWER;

    SendTransaction st;

    insertSendTransaction(op.Hdr().dlgId, &st);

    struct timeval tv, tv1;
    gettimeofday(&tv, NULL);

    pipe->sendPck(&op);
    st.responseEvent.Wait(m_Timeout);

    gettimeofday(&tv1, NULL);
    int t = (tv1.tv_sec - tv.tv_sec) * 1000 + (tv1.tv_usec - tv.tv_usec) / 1000;
    {
        MutexGuard mg(inUseLock);
        if(t > max_t) max_t = t;
        if(t < min_t) min_t = t;
        int total_t = time(NULL) - start_t;
        billcount++;
        smsc_log_debug(logger, " %d time to bill %d, max=%d, min=%d, avg=%d, persec=%d", op.Hdr().dlgId, t, max_t, min_t, total_t / billcount, billcount * 1000 / total_t);
    }

    deleteSendTransaction(op.Hdr().dlgId);

    return st.status;
}

void BillingManagerImpl::onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& pck)
{
    INPPacketAC* c = static_cast<INPPacketAC *>(pck.get());
    CsBillingHdr_dlg* hdr = static_cast<CsBillingHdr_dlg*>(c->pHdr());
    ChargeSmsResult* cmd = static_cast<ChargeSmsResult*>(c->pCmd());

     if(hdr->Id() != INPCSBilling::HDR_DIALOG)
        throw SCAGException("unsupported Inman packet header: %u", hdr->Id());

     if(cmd->Id() == INPCSBilling::CHARGE_SMS_RESULT_TAG)
     {
         try { 
             cmd->loadDataBuf();
             this->onChargeSmsResult(cmd, hdr);
         } catch (SerializerException& exc)
         {
             throw SCAGException("Corrupted cmd %u (dlgId: %u): %s",
                                 cmd->Id(), hdr->dlgId, exc.what());
         }
     } else throw SCAGException("Unknown command recieved: %u", cmd->Id());
}

void BillingManagerImpl::onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
    smsc_log_error(logger, "BillingManager error: Cannot receive command. Details: %s", p_exc->what());
    m_Connected = false;
    socket->Close();
}
#endif

void BillingManagerImpl::Start()
{
    #ifdef MSAG_INMAN_BILL
    MutexGuard guard(stopLock);
    if (!m_bStarted)
    {
        m_bStarted = true;
        Thread::Start();
    }
    #endif
}

void BillingManagerImpl::Stop()
{
    #ifdef MSAG_INMAN_BILL
    MutexGuard guard(stopLock);
    if(m_bStarted)
    {
        m_bStarted = false;
        socket->Close();
        connectEvent.Signal();
        exitEvent.Wait();
    }
    #endif
    smsc_log_info(logger,"BillingManager::stop");
}

int BillingManagerImpl::Execute()
{
    #ifdef MSAG_INMAN_BILL
    smsc_log_info(logger,"BillingManager::start executing");
    time_t prevCheck = time(NULL), lastReconnect = 0;
    while(isStarted())
    {
        try
        {
            if(m_Connected)
            {
                fd_set read;
                FD_ZERO( &read );
                FD_SET( socket->getSocket(), &read );

                struct timeval tv;
                tv.tv_sec = 10; 
                tv.tv_usec = 500;
                int n = select(socket->getSocket() + 1, &read, 0, 0, &tv);
                if(n < 0 || (n > 0 && pipe->onReadEvent()))
                    m_Connected = false;
            }
            else
            {
                connectEvent.Wait(10000);
                if(lastReconnect + m_ReconnectTimeout < time(NULL))
                {
                    m_Connected = Reconnect();
                    lastReconnect = time(NULL);
                }
            }

            if(time(NULL) > prevCheck + 10)
            {
                prevCheck = time(NULL);
                SendTransaction *st;
                MutexGuard mg(sendLock);
                if(SendTransactionHash.Count())
                {
                    smsc_log_debug(logger, "Processing expired inman transactions. Total: %d", SendTransactionHash.Count());
                    int key;
                    for(IntHash <SendTransaction *>::Iterator it = SendTransactionHash.First(); it.Next(key, st);)
                        if(st->lcmCtx && st->startTime + m_Timeout < prevCheck)
                            processAsyncResult(st);
                }
            }
        }catch (SCAGException& e)
        {
            smsc_log_error(logger, "BillingManager error: %s", e.what());
            m_Connected = false;
        }
    }

    ClearTransactions();
    smsc_log_info(logger,"BillingManager::stop executing");
    exitEvent.Signal();
    #endif
    return 0;
}

}
}
