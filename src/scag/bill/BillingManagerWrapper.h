/* "$Id$" */
//#define MSAG_INMAN_BILL

#ifndef _BILLING_MANAGER_WRAPPER_
#define _BILLING_BANAGER_WRAPPER_


#ifdef MSAG_INMAN_BILL
#include "inman/interaction/connect.hpp"
#include "inman/interaction/MsgBilling.hpp"
#endif

#include "core/network/Socket.hpp"
#include "logger/Logger.h"


namespace scag { namespace bill {

#ifdef MSAG_INMAN_BILL
using namespace smsc::inman::interaction;
#endif

using smsc::core::network::Socket;
using smsc::logger::Logger;

class BillingManagerWrapper 
#ifdef MSAG_INMAN_BILL
    : public SMSCBillingHandlerITF
#endif
{

    #ifdef MSAG_INMAN_BILL
    Socket * socket;
    Connect * pipe;
    #endif

    int m_Port;
    std::string m_Host;
protected:
    Logger * logger;

    BillingManagerWrapper() 
        : 
        #ifdef MSAG_INMAN_BILL
        socket(0), pipe(0), 
        #endif
        logger(Logger::getInstance("scag.BM")) 
    {

        #ifdef MSAG_INMAN_BILL
        socket = new Socket();

        pipe = new Connect(socket, Connect::frmLengthPrefixed, INPSerializer::getInstance(), logger);
        #endif
    };
    ~BillingManagerWrapper()
    {
        //if (socket) delete socket;

        #ifdef MSAG_INMAN_BILL
        if (pipe) delete pipe;
        #endif
    }
    virtual void InitConnection(std::string& host, int port)
    {
        #ifdef MSAG_INMAN_BILL
        m_Host = host;
        m_Port = port;
        #endif
    }
    virtual void receiveCommand(); //throws
    #ifdef MSAG_INMAN_BILL
    virtual void BillingManagerWrapper::sendCommand(INPPacketAC& op)
    {
        pipe->sendPck(&op);
    }
    #endif
    virtual bool Reconnect();
};



}}

#endif
