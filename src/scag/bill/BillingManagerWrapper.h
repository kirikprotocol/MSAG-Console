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

protected:
    Logger * logger;

    BillingManagerWrapper() : socket(0), pipe(0), logger(Logger::getInstance("scag.BM")) {};
    ~BillingManagerWrapper()
    {
        if (socket) delete socket;
        if (pipe) delete pipe;
    }
    virtual void initConnection(const char * host, int port);
    virtual void receiveCommand(); 
    virtual void BillingManagerWrapper::sendCommand(SerializableObject& op);
};



}}

#endif
