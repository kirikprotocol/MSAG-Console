#ifndef __SCAG_RULE_ENGINE_COMMAND_BRIGE__
#define __SCAG_RULE_ENGINE_COMMAND_BRIGE__

#include <scag/transport/smpp/SmppCommand.h>

namespace scag { namespace re {

using namespace scag::transport;
using namespace scag::transport::smpp;

class CommandBrige
{
public:
    static int getKey(const SCAGCommand& cmd)  
    {
        return 0;
    }

    static bool isFinalCommand(const SCAGCommand& command)  
    {
        SCAGCommand& _command = const_cast<SCAGCommand&>(command);
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) return false;

        _SmppCommand& cmd = *smppCommand->operator ->();


        CommandId cmdid = cmd.get_commandId();
        return (SUBMIT_RESP == cmdid);
    }

    static Address getDestAddr(const SCAGCommand& command)  
    {
        Address resultAddr;

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) return resultAddr;

        _SmppCommand& cmd = *smppCommand->operator ->();

        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;
        sms = (SMS*)dta;
        return sms->getDestinationAddress();
    }

    static Address getAbonentAddr(const SCAGCommand& command)  
    {
        Address resultAddr;

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) return resultAddr;

        _SmppCommand& cmd = *smppCommand->operator ->();

        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;

        if (!dta) return resultAddr;

        switch (cmdid) 
        {
        case DELIVERY:
            sms = (SMS*)dta;
            resultAddr = sms->originatingAddress;
            break;
        case SUBMIT:
            sms = (SMS*)dta;
            resultAddr = sms->destinationAddress;
            break;
        case DELIVERY_RESP:
            sms = ((SmsResp*)dta)->get_sms();
            resultAddr = sms->destinationAddress;
            break;
        case SUBMIT_RESP:
            sms = ((SmsResp*)dta)->get_sms();
            resultAddr = sms->originatingAddress;
            break;
        }

        return resultAddr;

    }

    static int16_t getUMR(const SCAGCommand& cmd)  
    {
//        Tag::SMPP_SAR_MSG_REF_NUM

        return 0;
    }
};


}}

#endif

