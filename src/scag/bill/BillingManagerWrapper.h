#ifndef _BILLING_MANAGER_WRAPPER_
#define _BILLING_BANAGER_WRAPPER_


#include "inman/interaction/messages.hpp"
#include "core/network/Socket.hpp"
#include "inman/interaction/connect.hpp"
#include "logger/Logger.h"


namespace scag { namespace bill {

using namespace smsc::inman::interaction;
using smsc::core::network::Socket;
using smsc::logger::Logger;

class BillingManagerWrapper : public SmscHandler
{

    Socket * socket;
    Connect * pipe;

    int m_Port;
    std::string m_Host;
protected:
    Logger * logger;

    BillingManagerWrapper() : socket(0), pipe(0), logger(Logger::getInstance("scag.BM")) 
    {
        socket = new Socket();
        pipe = new Connect(socket, SerializerInap::getInstance(), Connect::frmLengthPrefixed, logger);
    };
    ~BillingManagerWrapper()
    {
        if (socket) delete socket;
        if (pipe) delete pipe;
    }
    virtual void InitConnection(std::string& host, int port)
    {
        m_Host = host;
        m_Port = port;
    }
    virtual void receiveCommand(); 
    virtual void BillingManagerWrapper::sendCommand(SerializableObject& op);
    virtual bool Reconnect();
};



}}

#endif
