#include "CommandBridge.h"
// #include "scag/bill/base/BillingManager.h"
#include "scag/sessions/base/Session2.h"
#include "scag/transport/http/base/HttpCommandId.h"
#include "scag/util/encodings/Encodings.h"
#include "scag/util/HRTimer.h"
#include "util/recoder/recode_dll.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/transport/http/base/HttpCommand2.h"
#include "scag/transport/smpp/base/SmppCommand2.h"
#include "scag/re/base/SmppAdapter2.h"
#include "scag/sessions/base/Operation.h"

namespace scag2 {
namespace re {

// using namespace scag::transport::http;
using namespace sessions;
using namespace scag::util::encodings;
//using actions::CommandProperty;
 /*
namespace SMSTags 
{
    enum Tags
    {
        //Binary tags
        CHARGING = 0x4901,
        MESSAGE_TRANSPORT_TYPE = 0x4902,
        EXPECTED_MESSAGE_TRANSPORT_TYPE = 0x4904,

        //String tags
        MESSAGE_CONTENT_TYPE = 0x4903,
        EXPECTED_MESSAGE_CONTENT_TYPE = 0x4905
    };
}
   */
/*
whoisd-charging Byte    0x4901
whoisd-message-transport-type   Byte    0x4902
whoisd-message-content-type String  0x4903
whoisd-expected-message-transport-type  Byte    0x4904
whoisd-expected-message-content-type    String  0x4905

*/


EventHandlerType CommandBridge::getHTTPHandlerType(const SCAGCommand& command)
{
    switch(command.getCommandId())
    {
        case scag2::transport::http::HTTP_REQUEST:
            return EH_HTTP_REQUEST;
        case scag2::transport::http::HTTP_RESPONSE:
            return EH_HTTP_RESPONSE;
        case scag2::transport::http::HTTP_DELIVERY:
            return EH_HTTP_DELIVERY;
        default:
            return EH_UNKNOWN;
    }
}

EventHandlerType CommandBridge::getSMPPHandlerType(const SCAGCommand& command)
{
    EventHandlerType handlerType;

    SCAGCommand& _command = const_cast<SCAGCommand&>(command);
    SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);

    if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");

    
    // SMS& sms = getSMS(*smppCommand);
    CommandId cmdid = CommandId(smppCommand->getCommandId());

    //    CSmppDiscriptor SmppDiscriptor;
    // int receiptMessageId = 0;

    switch (cmdid) 
    {
    case DELIVERY:
        // receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
        // handlerType = receiptMessageId ? EH_RECEIPT : EH_DELIVER_SM;
        handlerType = EH_DELIVER_SM;
        break;

    case SUBMIT:
        handlerType = EH_SUBMIT_SM;
        break;

    case DELIVERY_RESP:
        // receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
        // handlerType = receiptMessageId ? EH_RECEIPT : EH_DELIVER_SM_RESP;
        handlerType = EH_DELIVER_SM_RESP;
        break;

    case SUBMIT_RESP:
        handlerType = EH_SUBMIT_SM_RESP;
        break;

    case DATASM:
        handlerType = EH_DATA_SM;
        break;

    case DATASM_RESP:
        handlerType = EH_DATA_SM_RESP;
        break;

    default:
        handlerType = EH_UNKNOWN;
    }
    return handlerType;
}


EventHandlerType CommandBridge::getHandlerType(const SCAGCommand& command)
{

    TransportType ttype = command.getType();
    EventHandlerType handlerType;

    switch (ttype) 
    {
    case SMPP:
        handlerType = getSMPPHandlerType(command);
        break;
    case HTTP:
        handlerType = getHTTPHandlerType(command);
        break;
    default:
        handlerType = EH_UNKNOWN;
    }

    return handlerType;
}


bool CommandBridge::hasMSB(const char* data, int len)
{
    while(len-- && !(data[len] & 0x80));
    return len >= 0;
}


std::string CommandBridge::getMessageBody(SmppCommand& command)
{
    return SmppCommand::getMessageBody(getSMS(command));
}


DataSmDirection CommandBridge::getPacketDirection(const SCAGCommand& command)
{
    SCAGCommand& _command = const_cast<SCAGCommand&>(command);

    SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
    if (!smppCommand) return dsdUnknown;

    CommandId cmdid = CommandId(smppCommand->getCommandId());

    switch (cmdid) 
    {
    case DELIVERY:
    case SUBMIT:
    case DATASM:
        return smppCommand->get_smsCommand().dir;

    case DELIVERY_RESP:
    case SUBMIT_RESP:
    case DATASM_RESP:
        return smppCommand->get_resp()->get_dir();
    default:
        break;
    }

    return dsdUnknown;
}


int CommandBridge::getProtocolForEvent(SCAGCommand& command)
{
    TransportType cmdType = command.getType();

    if (cmdType == SMPP) 
    {
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&command);
        SMS& sms = getSMS(*smppCommand);

        if (sms.hasIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP)) return PROTOCOL_SMPP_USSD;
        return PROTOCOL_SMPP_SMS;
    } 
    else if (cmdType == HTTP) {
        return PROTOCOL_HTTP;
    }
    else if (cmdType == MMS) {
        return PROTOCOL_MMS;
    }

    throw SCAGException("CommandBridge: Unknown command protocol");
}                 


actions::CommandProperty CommandBridge::getCommandProperty(SCAGCommand& command, const Address& abonentAddr, uint8_t operationType) {
  TransportType transport = command.getType();
  bill::Infrastructure& istr = bill::BillingManager::Instance().getInfrastructure();
  uint32_t operatorId = istr.GetOperatorID(abonentAddr);
  if (transport != SMPP && transport != HTTP) {
    throw SCAGException("getCommandProperty: unsupported transport type: %d. OperatorID=%d. Abonent addr=%s ",
                         transport, abonentAddr.toString().c_str());
  }

  if (transport == HTTP) {
    transport::http::HttpCommand& hc = (transport::http::HttpCommand&)command;
    uint8_t hi = getHTTPHandlerType(command);
    Property routeId;
    routeId.setInt(hc.getRouteId());
    return actions::CommandProperty(&command, 0, abonentAddr, hc.getProviderId(), operatorId,
                           hc.getServiceId(), -1, CO_HTTP_DELIVERY, routeId, hi);
  } else { //(transport == SMPP)
    transport::smpp::SmppCommand& smppcommand = static_cast<transport::smpp::SmppCommand&>(command);
    smpp::SmppCommandAdapter _command(smppcommand);
    int serviceId = command.getServiceId();
    int providerId = istr.GetProviderID(serviceId);
    uint8_t hi = getSMPPHandlerType(command);
    SMS& sms = getSMS(smppcommand);
    int msgRef = sms.hasIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE) ? sms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE):-1;
    Property routeId;
    routeId.setStr(sms.getRouteId());

    return actions::CommandProperty( &command, smppcommand.get_status(), abonentAddr, providerId, operatorId,
                            serviceId, msgRef,transport::CommandOperation(operationType), routeId, hi );
  }
}

void CommandBridge::CheckCommandProperty(SCAGCommand& command, const actions::CommandProperty& cp, const SessionPrimaryKey& primaryKey, Session* session) {
  TransportType transport = command.getType();
  if (transport == HTTP) {
    transport::http::HttpCommand& hc = (transport::http::HttpCommand&)command;
    if (cp.operatorId == 0) {
      RegisterAlarmEvent( 1, cp.abonentAddr.toString(), sessions::PROTOCOL_HTTP, hc.getServiceId(),
                          hc.getProviderId(), 0, 0, primaryKey,
                          hc.getCommandId() == transport::http::HTTP_RESPONSE ? 'O' : 'I');

      throw SCAGException("CheckCommandProperty: Cannot find OperatorID for %s abonent", cp.abonentAddr.toString().c_str());
    }
  } else { // transport == SMPP
    transport::smpp::SmppCommand& smppcommand = static_cast<transport::smpp::SmppCommand&>(command);
    if (cp.providerId == 0) {
        if ( smppcommand.isResp() && session ) session->closeCurrentOperation();
        throw SCAGException("CheckCommandProperty: Cannot find ProviderID for ServiceID=%d", cp.serviceId);
    }
    if (cp.operatorId == 0) {
        RegisterAlarmEvent(1, cp.abonentAddr.toString(), getProtocolForEvent(smppcommand), cp.serviceId,
                           cp.providerId, 0, 0, primaryKey,
                           (cp.handlerId == EH_SUBMIT_SM)||(cp.handlerId == EH_DELIVER_SM) ? 'I' : 'O');
        
        if ( smppcommand.isResp() && session ) session->closeCurrentOperation();
        throw SCAGException("CheckCommandProperty: Cannot find OperatorID for %s abonent", cp.abonentAddr.toString().c_str());
    }
  }
}

void CommandBridge::RegisterTrafficEvent(const actions::CommandProperty& commandProperty,
                                         const sessions::SessionPrimaryKey& sessionPrimaryKey,
                                         const std::string& messageBody,
                                         util::HRTiming* hrt )
{
    SaccTrafficInfoEvent* ev = new SaccTrafficInfoEvent();
    if (hrt) hrt->mark("ev.newsacc");

    uint8_t handlerId = commandProperty.handlerId; 

    ev->Header.cCommandId = handlerId;
    ev->Header.cProtocolId = commandProperty.protocol;
    ev->Header.iServiceId = commandProperty.serviceId;
    ev->Header.iServiceProviderId = commandProperty.providerId; 
    
    timeval tv;
    gettimeofday(&tv,0);

    ev->Header.lDateTime = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    ev->Header.pAbonentNumber = commandProperty.abonentAddr.toString().c_str();
    ev->Header.sCommandStatus = commandProperty.status;
    ev->Header.iOperatorId = commandProperty.operatorId;
    
    if (handlerId == EH_SUBMIT_SM || handlerId == EH_DELIVER_SM || handlerId == EH_DATA_SM) {
      ev->pMessageText.append(messageBody.data(), messageBody.size());
    }

    ev->Header.pSessionKey = sessionPrimaryKey.toString();

    if (commandProperty.direction == dsdSc2Srv || handlerId == EH_HTTP_REQUEST || handlerId == EH_HTTP_DELIVERY) {
      ev->cDirection = 'I';
    } else {
      ev->cDirection = 'O';
    }

    if ( !commandProperty.keywords.empty() ) {
      ev->keywords.append(commandProperty.keywords.data(), commandProperty.keywords.size());
    }

    if (hrt) hrt->mark("ev.fillsac");
    Statistics::Instance().registerSaccEvent( ev );
    if (hrt) hrt->mark("ev.regsac");
}

void CommandBridge::RegisterAlarmEvent(uint32_t eventId, const std::string& addr, uint8_t protocol,
                                       uint32_t serviceId, uint32_t providerId, uint32_t operatorId,
                                       uint16_t commandStatus, const sessions::SessionPrimaryKey& sessionPrimaryKey, char dir)
{
    SaccAlarmEvent* ev = new SaccAlarmEvent();

    ev->Header.cCommandId = 3;
    ev->Header.cProtocolId = protocol;
    ev->Header.iServiceId = serviceId;
    ev->Header.iServiceProviderId = providerId; 
    ev->Header.pAbonentNumber = addr;
    ev->Header.sCommandStatus = commandStatus;
    ev->Header.iOperatorId = operatorId;
    ev->iAlarmEventId = eventId;
    
    timeval tv;
    gettimeofday(&tv,0);
    ev->Header.lDateTime = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    ev->Header.pSessionKey = sessionPrimaryKey.toString();
    ev->cDirection = dir;

    Statistics::Instance().registerSaccEvent(ev);
}

void AssignAddress(Address& address, const char * str)
{
    Address addr(str);
    address = addr;
}


}}


