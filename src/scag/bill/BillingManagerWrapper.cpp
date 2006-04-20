#include "BillingManagerWrapper.h"

#include "scag/exc/SCAGExceptions.h"

#include "inman/common/console.hpp"
#include "inman/common/util.hpp"
#include "logger/Logger.h"


namespace scag { namespace bill {

using smsc::inman::common::Console;
using smsc::inman::common::format;
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


    return true;
}

void BillingManagerWrapper::receiveCommand()
{
    fd_set read;
    FD_ZERO( &read );
    FD_SET( socket->getSocket(), &read );

    struct timeval tv;
    tv.tv_sec = 0; 
    tv.tv_usec = 500;

    int n = select(socket->getSocket()+1, &read, 0, 0, &tv);

    if( n > 0 )
    {
        SmscCommand* cmd = static_cast<SmscCommand*>(pipe->receiveObj());

        if (cmd) 
        {
             if (cmd->getObjectId() == smsc::inman::interaction::CHARGE_SMS_RESULT_TAG) 
             {
                 try { 
                     cmd->loadDataBuf(); 
                     cmd->handle(this);
                 } catch (SerializerException& exc) 
                 {
                     throw SCAGException("Corrupted cmd %u (dlgId: %u): %s",cmd->getObjectId(), cmd->getDialogId(),exc.what());
                 }
             } else throw SCAGException("Unknown command recieved: %u",cmd->getObjectId());

        } 
     } ///else 
        //Reconnect();
    
}


void BillingManagerWrapper::sendCommand(SerializableObject& op)
{
    pipe->sendObj(&op);
}

}}
