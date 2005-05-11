#indef __SME_T_H__
#define __SME_T_H__

#include <stdio.h>
#include "sme/SmppBase.hpp"
#include "sms/sms.h"
#include <unistd.h>
#include "util/smstext.h"
#include <logger/Logger.h>
#include "core/synchronization/Event.hpp"

using namespace smsc::sms;
using namespace smsc::sme;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;
using namespace smsc::util;
using namespace smsc::logger;
using namespace smsc::core::threads;

extern bool stopProcess;

/**
Listener to SME for USSD processing.
Main trend is RX message are moved to TX message.
*/
class MyListener:public SmppPduEventListener{
public:
  void handleEvent(SmppHeader *pdu);
  void handleError(int errorCode);
  void setTrans(SmppTransmitter *t);
protected:
  SmppTransmitter* trans;
};

class UssdSmeRunner:public Thread
{
public:
 int Execute();
};


