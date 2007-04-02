/* "$Id$" */
#include "BillingManagerWrapper.h"

#include "scag/exc/SCAGExceptions.h"

#ifdef MSAG_INMAN_BILL
#include "inman/common/console.hpp"
#endif

#include "logger/Logger.h"


namespace scag { namespace bill {

#ifdef MSAG_INMAN_BILL
using smsc::inman::common::Console;
using smsc::inman::interaction;
#endif

using scag::exceptions::SCAGException;

/*
void BillingManagerWrapper::initConnection(const char * host, int port)
{
    socket = new Socket();

    if (socket->Init(host, port, 1000)) {
        throw SCAGException("can't init socket to BillingServer on host '%s', port '%d': error %s (%d)\n", host, port, strerror(errno), errno);
    }

    if (socket->Connect()) {
        throw SCAGException("can't connect socket to BillingServer on host '%s', port '%d' : %s (%d)\n", host, port, strerror(errno), errno);
    }
  
    pipe = new Connect(socket, SerializerInap::getInstance(), Connect::frmLengthPrefixed, logger);
}
*/

bool BillingManagerWrapper::Reconnect()
{
    #ifdef MSAG_INMAN_BILL

    if (socket->Init(m_Host.c_str(), m_Port, 1000)) 
    {
        smsc_log_warn(logger, "Can't init socket to BillingServer on host '%s', port '%d': error %s (%d)\n", m_Host.c_str(), m_Port, strerror(errno), errno);
        return false;
    }

    if (socket->Connect())
    {
        smsc_log_warn(logger, "Can't connect socket to BillingServer on host '%s', port '%d' : %s (%d)\n", m_Host.c_str(), m_Port, strerror(errno), errno);
        return false;
    }        
    #endif

    return true;
}

#ifdef MSAG_INMAN_BILL
void BillingManagerWrapper::onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& pck)
{
    INPPacketAC * cmd = static_cast<INPPacketAC *>(pck.get());
    if ((cmd->pHdr())->Id() != INPCSBilling::HDR_DIALOG)
        throw SCAGException("unsupported Inman packet header: %u", (cmd->pHdr())->Id());

    CsBillingHdr_dlg * hdr = static_cast<CsBillingHdr_dlg*>(cmd->pHdr());

     if ((cmd->pCmd())->Id() == INPCSBilling::CHARGE_SMS_RESULT_TAG)
     {
         try { 
             (cmd->pCmd())->loadDataBuf();
             this->onChargeSmsResult(static_cast<ChargeSmsResult*>(cmd->pCmd()), hdr);
         } catch (SerializerException& exc)
         {
             throw SCAGException("Corrupted cmd %u (dlgId: %u): %s",
                                 (cmd->pCmd())->Id(), hdr->dlgId, exc.what());
         }
     } else throw SCAGException("Unknown command recieved: %u", (cmd->pCmd())->Id());
}

void BillingManagerWrapper::onConnectError(Connect* conn, std::auto_ptr<CustomException>& p_exc)
{
    error_msg = "Connect error: ";
    error_msg += p_exc->what();
}
#endif

void BillingManagerWrapper::receiveCommand()
{
    #ifdef MSAG_INMAN_BILL

    fd_set read;
    FD_ZERO( &read );
    FD_SET( socket->getSocket(), &read );

    struct timeval tv;
    tv.tv_sec = 0; 
    tv.tv_usec = 500;

    error_msg = "";

    int n = select(socket->getSocket()+1, &read, 0, 0, &tv);

    if( n > 0 ) pipe->onReadEvent();

    if(error_msg.length() > 0)
        throw SCAGException(error_msg.c_str());
        //Reconnect();
    #endif
}

}}
