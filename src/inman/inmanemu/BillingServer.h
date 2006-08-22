#ifndef _INMANEMU_BILL_BILLING_SERVER_
#define _INMANEMU_BILL_BILLING_SERVER_

#include <logger/Logger.h>
#include "BillProcessor.h"
//#include <inman/storage/CDRStorage.hpp>
#include "inman/interaction/messages.hpp"
#include "core/network/Socket.hpp"

namespace inmanemu { namespace server {

using namespace smsc::inman::interaction;
using namespace smsc::core::network;
using namespace inmanemu::processor;
//using smsc::inman::filestore::InBillingFileStorage;

using smsc::logger::Logger;

class BillingServer
{
    Logger * logger;
    bool needToStop;
    ObjectBuffer buff;

    Socket socket;
    BillProcessor processor;
    bool m_ClientConnected;
    //InBillingFileStorage * fileStorage;

    Socket* clnt;

    bool isStarted()
    {
        return !needToStop;
    }

    SerializableObject * ReadCommand();
    ChargeSmsResult * CreateRespOnCharge(SerializableObject * obj);
    void SendResp(SerializableObject * resp);
    bool ClientConnected();
    void ProcessResultCommand(SerializableObject * resp);


public:
   void Init(const std::string& host, int port, const std::string& cdr_dir);
   void Run();
   void Stop();
   BillingServer();
   ~BillingServer();
};


}}

#endif

