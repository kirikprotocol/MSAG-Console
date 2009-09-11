#ifndef SMSC_INFOSME_INFOSMEPDULISTENER
#define SMSC_INFOSME_INFOSMEPDULISTENER

#include <string>
#include "sms/sms.h"
#include "sme/SmppBase.hpp"
#include "logger/Logger.h"

// #include "TaskProcessor.h"

namespace smsc {
namespace infosme {

using namespace smsc::smpp;
using namespace smsc::sme;
using smsc::logger::Logger;

class SmscConnector;

class InfoSmePduListener: public smsc::sme::SmppPduEventListener 
{
public:
    InfoSmePduListener( SmscConnector& proc,
                        Logger* log) :
    processor(proc),
    syncTransmitter(0), asyncTransmitter(0),
    logger(log) {};
  virtual ~InfoSmePduListener() {}; 

  void setSyncTransmitter(SmppTransmitter *transmitter);
  void setAsyncTransmitter(SmppTransmitter *transmitter);
  void processReceipt (SmppHeader *pdu);
  void processResponce(SmppHeader *pdu);
  void handleEvent(SmppHeader *pdu);
  void handleError(int errorCode);

private:
    SmscConnector& processor;
  SmppTransmitter* syncTransmitter;
  SmppTransmitter* asyncTransmitter;
  std::string smscId_;
  Logger* logger;
};

}//infosme
}//smsc

#endif
