//#define MSAG_INMAN_BILL

#ifndef _BILLING_MANAGER_WRAPPER_
#define _BILLING_BANAGER_WRAPPER_


#ifdef MSAG_INMAN_BILL
#include "inman/interaction/messages.hpp"
#include "inman/interaction/connect.hpp"
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
    : public SmscHandler
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
        pipe = new Connect(socket, SerializerInap::getInstance(), Connect::frmLengthPrefixed, logger);
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
    virtual void receiveCommand(); 
    #ifdef MSAG_INMAN_BILL
    virtual void BillingManagerWrapper::sendCommand(SerializableObject& op)
    {
        pipe->sendObj(&op);
    }
    #endif
    virtual bool Reconnect();
};



}}

#endif
