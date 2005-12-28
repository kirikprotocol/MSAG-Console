#ifndef __SCAG_RULE_ENGINE_COMMAND_BRIGE__
#define __SCAG_RULE_ENGINE_COMMAND_BRIGE__

#include <scag/transport/smpp/SmppCommand.h>
#include <scag/exc/SCAGExceptions.h>

namespace scag { namespace re {

using namespace scag::transport;
using namespace scag::transport::smpp;
using namespace scag::exceptions;

enum CommandOperations
{
    CO_DELIVER_SM,
    CO_DELIVER_SM_RESP,
    CO_SUBMIT_SM,
    CO_SUBMIT_SM_RESP,
    CO_RECEIPT_DELIVER_SM,
    CO_RECEIPT_DELIVER_SM_RESP
};

struct CSmppDiscriptor
{
    CommandOperations cmdType;
    int currentIndex;
    int lastIndex;
};

class CommandBrige
{
public:

    static CSmppDiscriptor getSmppDiscriptor(const SCAGCommand& command)
    {

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");
        
        
        _SmppCommand& cmd = *smppCommand->operator ->();


        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;
        CSmppDiscriptor SmppDiscriptor;
        int receiptMessageId = 0;

        if (!dta) throw SCAGException("Command Bridge Error: SCAGCommand data is invalid");

        switch (cmdid) 
        {
        case DELIVERY:
            sms = (SMS*)dta;
            receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

            if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT_DELIVER_SM;
            else SmppDiscriptor.cmdType = CO_DELIVER_SM;
            
            break;
        case SUBMIT:
            sms = (SMS*)dta;

            SmppDiscriptor.cmdType = CO_SUBMIT_SM;
            break;
        case DELIVERY_RESP:
            sms = ((SmsResp*)dta)->get_sms();

            receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

            if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT_DELIVER_SM_RESP;
            else SmppDiscriptor.cmdType = CO_DELIVER_SM_RESP;
            break;
        case SUBMIT_RESP:
            SmppDiscriptor.cmdType = CO_SUBMIT_SM_RESP;
            break;
        }

        if (sms == 0) throw SCAGException("Command Bridge Error: Unknown SCAGCommand data");

        SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);


        //TODO: Implement
        SmppDiscriptor.cmdType = CO_DELIVER_SM;

        return SmppDiscriptor;
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
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");


        _SmppCommand& cmd = *smppCommand->operator ->();

        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;

        if (!dta) throw SCAGException("Command Bridge Error: SCAGCommand data is invalid");

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

