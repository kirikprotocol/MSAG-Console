#ifndef _INMANEMU_BILL_BILLING_SERVER_
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _INMANEMU_BILL_BILLING_SERVER_

#include "logger/Logger.h"
#include "core/network/Socket.hpp"
#include "inman/inmanemu/BillProcessor.h"
//#include "inman/interaction/msgbill/MsgBilling.hpp"
#include "inman/interaction/msgbill/SmBillRequestMsg.hpp"

namespace inmanemu {
namespace server {

using smsc::logger::Logger;
using namespace smsc::inman::interaction;
using namespace smsc::core::network;
using namespace inmanemu::processor;



class BillingServer
{
    volatile bool needToStop;
    PacketBuffer_T<2048> buff;
    Socket socket;
    BillProcessor processor;
    bool m_ClientConnected;
    Socket * clnt;
    Logger * logger;

    bool isStarted(void) const { return !needToStop; }

    SmBillRequestMsg * ReadCommand(); //throws
    SPckChargeSmsResult * CreateRespOnCharge(SPckChargeSms * pck);
    void SendResp(SPckChargeSmsResult * pck);
    bool ClientConnected();
    void ProcessResultCommand(SPckDeliverySmsResult * pck);


public:
  static const INPBilling  _protoDef; //provided protocol definition

   void Init(const std::string& host, int port, const std::string& cdr_dir);
   void Run();
   void Stop();

   BillingServer() : needToStop(false), m_ClientConnected(false), clnt(0)
    , logger(smsc::logger::Logger::getInstance("BillingServer"))
   { }
   //
   ~BillingServer()
   { }
};


}}

#endif /* _INMANEMU_BILL_BILLING_SERVER_ */

