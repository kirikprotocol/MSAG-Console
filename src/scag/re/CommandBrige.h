#ifndef __SCAG_RULE_ENGINE_COMMAND_BRIGE__
#define __SCAG_RULE_ENGINE_COMMAND_BRIGE__

#include <scag/transport/smpp/SmppCommand.h>
#include <scag/exc/SCAGExceptions.h>
#include "scag/stat/Statistics.h"

namespace scag { namespace sessions {

struct CSessionPrimaryKey;

}}


namespace scag { namespace re {

using namespace scag::transport;
using namespace scag::transport::smpp;
using namespace scag::exceptions;

using namespace scag::stat;
using namespace smsc::smpp::UssdServiceOpValue;

enum CommandOperations
{
    CO_DELIVER_SM,
    CO_DELIVER_SM_RESP,
    CO_SUBMIT_SM,
    CO_SUBMIT_SM_RESP,
    CO_RECEIPT_DELIVER_SM,
    CO_RECEIPT_DELIVER_SM_RESP,

    CO_USSD_DELIVER,
    CO_USSD_DELIVER_RESP,

    CO_USSD_SUBMIT,
    CO_USSD_SUBMIT_RESP
};

enum EventHandlerType
{
    EH_UNKNOWN,
    EH_SUBMIT_SM,
    EH_SUBMIT_SM_RESP,
    EH_DELIVER_SM,
    EH_DELIVER_SM_RESP,
    EH_RECEIPT
};

struct CSmppDiscriptor
{
    CommandOperations cmdType;
    int currentIndex;
    int lastIndex;
    bool isUSSDClosed;
    bool wantOpenUSSD;

    CSmppDiscriptor() : currentIndex(0), lastIndex(0), isUSSDClosed(false), wantOpenUSSD(false) {}
    
};



class CommandBrige
{
public:

    static std::string getMessageBody(SmppCommand& command);
    static void makeTrafficEvent(SmppCommand& command, int handlerType, scag::sessions::CSessionPrimaryKey& sessionPrimaryKey, SACC_TRAFFIC_INFO_EVENT_t& ev);
    static EventHandlerType getHandlerType(const SCAGCommand& command);
    static CSmppDiscriptor getSmppDiscriptor(const SCAGCommand& command);

    //static void makeBillEvent(int serviceId, std::string& abonentAddr, int billCommand, SACC_BILLING_INFO_EVENT_t& ev);

    /*
    static int ExtractCommandType(SCAGCommand& command)
    {
        SmppCommand * smpp = dynamic_cast<SmppCommand*>(&command);
        if (smpp) 
        {
            _SmppCommand * cmd = smpp->operator ->();
            if (!cmd) return 0;
            return cmd->get_commandId();
        }
    
        return 0;
    }            */



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
            sms = command->get_sms();
            break;
        case DELIVERY_RESP:
        case SUBMIT_RESP:
            SmsResp * resp = command->get_resp();
            if (resp) sms = resp->get_sms();
            break;
        }

        if (!sms) throw SCAGException("Command Bridge Error: Cannot get SMS from SmppCommand");
        return *sms;
    }

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
            resultAddr = sms.destinationAddress;
            break;
        case SUBMIT_RESP:
            resultAddr = sms.originatingAddress;
            break;
        }
  
        return resultAddr;

    }



    static int16_t getUMR(const SCAGCommand& command)  
    {
//        Tag::SMPP_SAR_MSG_REF_NUM

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");


        SMS& sms = getSMS(*smppCommand);

        return sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
    }
};


}}

#endif

