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
    //SMS
    CO_DELIVER,
    CO_SUBMIT,
    CO_RECEIPT,
    CO_DATA_SC_2_SME,
    CO_DATA_SC_2_SC,
    CO_DATA_SME_2_SME,
    CO_DATA_SME_2_SC,

    //USSD
    CO_USSD_DIALOG,

    //HTTP
    CO_HTTP_DELIVERY

};

enum EventHandlerType
{
    EH_UNKNOWN,
    EH_SUBMIT_SM,
    EH_SUBMIT_SM_RESP,
    EH_DELIVER_SM,
    EH_DELIVER_SM_RESP,
    EH_RECEIPT,

    EH_HTTP_REQUEST,
    EH_HTTP_RESPONSE,
    EH_HTTP_DELIVERY,

    EH_DATA_SM,
    EH_DATA_SM_RESP
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



class CommandBrige
{
private:
    //static int getProtocolForEvent(SmppCommand& command);
    static EventHandlerType getSMPPHandlerType(const SCAGCommand& command);
    static EventHandlerType getHTTPHandlerType(const SCAGCommand& command);

public:

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
                resultAddr = sms.destinationAddress;
                break;
            case dsdSrv2Sc:
                resultAddr = sms.originatingAddress;
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



    static int16_t getUMR(const SCAGCommand& command)  
    {
//        Tag::SMPP_SAR_MSG_REF_NUM

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");


        SMS& sms = getSMS(*smppCommand);

        if (sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM)) 
            return sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);

        return 0;
    }
};


}}

#endif

