#ifndef __SCAG_RULE_ENGINE_COMMAND_BRIGE__
#define __SCAG_RULE_ENGINE_COMMAND_BRIGE__

//#include "scag/transport/SCAGCommand.h"
#include "scag/transport/smpp/SmppCommand.h"
#include <scag/exc/SCAGExceptions.h>
#include "scag/stat/Statistics.h"

/*namespace scag { namespace sessions {

struct CSessionPrimaryKey;

}}*/

namespace scag { namespace re {

using namespace scag::transport;
using namespace scag::transport::smpp;
using namespace scag::exceptions;

using namespace scag::stat;
using namespace smsc::smpp::UssdServiceOpValue;
using smsc::sms::Address;

enum CommandOperations
{
    //SMS
    CO_DELIVER = 0,
    CO_SUBMIT = 1,
    CO_RECEIPT = 2,
    CO_DATA_SC_2_SME = 3,
    CO_DATA_SC_2_SC = 4,
    CO_DATA_SME_2_SME = 5,
    CO_DATA_SME_2_SC = 6,

    //USSD
    CO_USSD_DIALOG = 7,
    CO_PULL_USSD_DIALOG = 7,
    CO_PUSH_USSD_DIALOG = 8,

    //HTTP
    CO_HTTP_DELIVERY = 9

};

enum EventHandlerType
{
    EH_UNKNOWN = 0,
    EH_SUBMIT_SM = 1,
    EH_SUBMIT_SM_RESP = 2,
    EH_DELIVER_SM = 3,
    EH_DELIVER_SM_RESP = 4,
    EH_RECEIPT = 5,

    EH_HTTP_REQUEST = 6,
    EH_HTTP_RESPONSE = 7,
    EH_HTTP_DELIVERY = 8,

    EH_DATA_SM = 9,
    EH_DATA_SM_RESP = 10,

    EH_SESSION_INIT = 11,
    EH_SESSION_DESTROY = 12
};

struct CSmppDiscriptor
{
    CommandOperations cmdType;
    int currentIndex;
    int lastIndex;

    bool isUSSDClosed;
    bool wantOpenUSSD;

    bool m_waitReceipt;
    bool isResp;

    CSmppDiscriptor() : 
        currentIndex(0), 
        lastIndex(0), 
        isUSSDClosed(false), 
        wantOpenUSSD(false), 
        m_waitReceipt(false),
        isResp(false) {}
    
};


void AssignAddress(Address& address, const char * str);


class CommandBrige
{
private:
    //static int getProtocolForEvent(SmppCommand& command);
    static EventHandlerType getSMPPHandlerType(const SCAGCommand& command);
    static EventHandlerType getHTTPHandlerType(const SCAGCommand& command);

public:
    static DataSmDirection getPacketDirection(const SCAGCommand& command);

    static std::string getMessageBody(SmppCommand& command);
    //static void makeTrafficEvent(SmppCommand& command, int handlerType, scag::sessions::CSessionPrimaryKey& sessionPrimaryKey, SACC_TRAFFIC_INFO_EVENT_t& ev);
    static EventHandlerType getHandlerType(const SCAGCommand& command);
    static CSmppDiscriptor getSmppDiscriptor(const SCAGCommand& command);

    static int getProtocolForEvent(SCAGCommand& command);


    static Address getDestAddr(const SCAGCommand& command)  
    {
        Address resultAddr;

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: Cannot get DestAddress - SCAGCommand is not smpp-type");

        SMS& sms = getSMS(*smppCommand);
        return sms.getDestinationAddress();
    }

    static SMS& getSMS(SmppCommand& command)
    {
        CommandId cmdid = command->get_commandId();

        SMS * sms = 0;

        switch (cmdid) 
        {
        case DELIVERY:
        case SUBMIT:
        case DATASM:
            sms = command->get_sms();
            break;
        case DELIVERY_RESP:
        case SUBMIT_RESP:
        case DATASM_RESP:
            SmsResp * resp = command->get_resp();
            if (resp) sms = resp->get_sms();
            break;
        }

        if (!sms) throw SCAGException("Command Bridge Error: Cannot get SMS from SmppCommand");
        return *sms;
    }
/*
    static void setAbonentAddr(SmppCommand& command)
    {
        switch (command->get_smsCommand().dir)
        {
        case dsdSc2Srv:
            AssignAddress(sms->destinationAddress, property.getStr().c_str());
            break;
        case dsdSrv2Sc:
            AssignAddress(sms->originatingAddress, property.getStr().c_str());
            break;
        }
    }
*/
    static Address getAbonentAddr(SmppCommand& command)  
    {
        Address resultAddr;

        SMS& sms = getSMS(command);
        CommandId cmdid = command->get_commandId();

        switch (cmdid) 
        {
        case DELIVERY:
            resultAddr = sms.originatingAddress;
            break;
        case SUBMIT:
            resultAddr = sms.destinationAddress;
            break;
        case DELIVERY_RESP:
            resultAddr = sms.originatingAddress;
            break;
        case SUBMIT_RESP:
            resultAddr = sms.destinationAddress;
            break;
        case DATASM:
            switch (command->get_smsCommand().dir)
            {
            case dsdSc2Srv:
                resultAddr = sms.originatingAddress;
                break;
            case dsdSrv2Sc:
                resultAddr = sms.destinationAddress;
                break;
            }

            break;
        case DATASM_RESP:

            if (!command->get_resp()) break;

            switch (command->get_resp()->get_dir())
            {
            case dsdSc2Srv:
                resultAddr = sms.destinationAddress;
                break;
            case dsdSrv2Sc:
                resultAddr = sms.originatingAddress;
                break;
            }
            break;
        }
  
        return resultAddr;

    }



/*    static int16_t getUMR(const SCAGCommand& command)  
    {
//        Tag::SMPP_SAR_MSG_REF_NUM

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");


        SMS& sms = getSMS(*smppCommand);

        if (sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM)) 
            return sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);

        return 0;
    }*/
};


}}

#endif

