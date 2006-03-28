#ifndef __SCAG_RULE_ENGINE_COMMAND_BRIGE__
#define __SCAG_RULE_ENGINE_COMMAND_BRIGE__

#include <scag/transport/smpp/SmppCommand.h>
#include <scag/exc/SCAGExceptions.h>
#include "util/recoder/recode_dll.h"
#include "scag/stat/Statistics.h"
//#include "smpp/smpp_structure.h"

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
};



class CommandBrige
{
public:

    static std::string getMessageBody(SmppCommand& command)
    {
        unsigned len1, len2, len;

        const char * buff1;
        const char * buff2;
        const char * buff;

        std::string str;
        char ucs2buff[2048];
    /*
        static const uint8_t SMSC7BIT             = 0;
        static const uint8_t LATIN1               = 3;
        static const uint8_t BINARY               = BIT(2);
        static const uint8_t UCS2                 = BIT(3);
    */
        SMS& data = getSMS(command);
        
        buff1 = data.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len1);
        buff2 = data.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len2);

        if (len1>len2) {
            len = len1;
            buff = buff1;
        } 
        else {
            len = len2;
            buff = buff2;
        }

        int code = data.getIntProperty(Tag::SMPP_DATA_CODING);

        switch (code) 
        {
        case smsc::smpp::DataCoding::SMSC7BIT:
            Convert7BitToUCS2(buff, len, (short *)ucs2buff, len*2); 
            str.assign(ucs2buff,len*2);

            ucs2buff[0] = 0;
            ucs2buff[1] = 0;
            str.append(ucs2buff,2);
            break;
        case smsc::smpp::DataCoding::LATIN1:
            ConvertMultibyteToUCS2(buff, len, (short *)ucs2buff, len*2, CONV_ENCODING_KOI8R);
            str.assign(ucs2buff,len*2);

            ucs2buff[0] = 0;
            ucs2buff[1] = 0;
            str.append(ucs2buff,2);
            break;
        default:
    //        memcpy(ucs2buff, buff, len);
            str.assign(buff,len);
            ucs2buff[0] = 0;
            ucs2buff[1] = 0;
            str.append(ucs2buff,2);
        }

        return str;
    }


    static void makeTrafficEvent(SmppCommand& command, int handlerType, scag::sessions::CSessionPrimaryKey& sessionPrimaryKey, SACC_TRAFFIC_INFO_EVENT_t& ev);
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


    static EventHandlerType getHandlerType(const SCAGCommand& command)
    {
        EventHandlerType handlerType = EH_UNKNOWN;

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");


        _SmppCommand& cmd = *smppCommand->operator ->();

        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;
        CSmppDiscriptor SmppDiscriptor;
        int receiptMessageId = 0;

        if (!dta) throw SCAGException("Command Bridge Error: Cannot get HandlerType - SCAGCommand data is invalid");

        switch (cmdid) 
        {
        case DELIVERY:
            sms = (SMS*)dta;
            receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

            if (receiptMessageId) handlerType = EH_RECEIPT;
            else handlerType = EH_DELIVER_SM;
            break;

        case SUBMIT:
            sms = (SMS*)dta;

            handlerType = EH_SUBMIT_SM;
            break;

        case DELIVERY_RESP:
            sms = ((SmsResp*)dta)->get_sms();

            if (sms) receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
            else throw SCAGException("Command Bridge Error: Cannot get HandlerType - SCAGCommand SMS data for DELIVERY_RESP is invalid");

            if (receiptMessageId) handlerType = EH_RECEIPT;
            else handlerType = EH_DELIVER_SM_RESP;
            break;

        case SUBMIT_RESP:
            handlerType = EH_SUBMIT_SM_RESP;
            break;
        }

        return handlerType;
    }



    static CSmppDiscriptor getSmppDiscriptor(const SCAGCommand& command)
    {

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: Cannot get SmppDiscriptor - SCAGCommand is not smpp-type");
        
        
        _SmppCommand& cmd = *smppCommand->operator ->();

        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;
        CSmppDiscriptor SmppDiscriptor;
        int receiptMessageId = 0;
        SmppDiscriptor.isUSSDClosed = false;

        if (!dta) throw SCAGException("Command Bridge Error: Cannot get SmppDiscriptor - SCAGCommand data is invalid");

        switch (cmdid) 
        {
        case DELIVERY:
            sms = (SMS*)dta;
            receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

            if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT_DELIVER_SM;
            else if (sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
            {
                SmppDiscriptor.cmdType = CO_USSD_DELIVER;
                SmppDiscriptor.isUSSDClosed = (sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSR_REQUEST);
            }
            else SmppDiscriptor.cmdType = CO_DELIVER_SM;

            
            SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
            SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
            
            break;
        case SUBMIT:
            sms = (SMS*)dta;

            if (sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
            {
                SmppDiscriptor.cmdType = CO_USSD_DELIVER_RESP;
                SmppDiscriptor.isUSSDClosed = (sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_RESPONSE);
            }
            else SmppDiscriptor.cmdType = CO_SUBMIT_SM;

            SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
            SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

            break;
        case DELIVERY_RESP:
            
            sms = ((SmsResp*)dta)->get_sms();

            if (sms) receiptMessageId = atoi(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
            else throw SCAGException("Command Bridge Error: Cannot get SmppDiscriptor - SCAGCommand SMS data for DELIVERY_RESP is invalid");

            SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
            SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

            if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT_DELIVER_SM_RESP;
            else if (sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) SmppDiscriptor.cmdType = CO_USSD_DELIVER_RESP;
            else SmppDiscriptor.cmdType = CO_DELIVER_SM_RESP;

            break;
        case SUBMIT_RESP:
            if (sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) SmppDiscriptor.cmdType = CO_USSD_DELIVER_RESP;
            else SmppDiscriptor.cmdType = CO_SUBMIT_SM_RESP;

            SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
            SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
            break;
        }
        /*
        if (sms == 0) throw SCAGException("Command Bridge Error: Unknown SCAGCommand data");

        SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
          */

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

    static SMS& getSMS(SmppCommand& command)
    {
        _SmppCommand& cmd = *command.operator->();

        CommandId cmdid = cmd.get_commandId();
        void * dta = cmd.dta;
        SMS * sms = 0;

        if (!dta) throw SCAGException("Command Bridge Error: Cannot get SMS from SmppCommand");

        switch (cmdid) 
        {
        case DELIVERY:
            sms = (SMS*)dta;
            break;
        case SUBMIT:
            sms = (SMS*)dta;
            break;
        case DELIVERY_RESP:
            sms = ((SmsResp*)dta)->get_sms();
            break;
        case SUBMIT_RESP:
            sms = ((SmsResp*)dta)->get_sms();
            break;
        }

        return *sms;
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

        if (!dta) throw SCAGException("Command Bridge Error: Cannot get AbonentAddress - SCAGCommand data is invalid");

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

    static int16_t getUMR(const SCAGCommand& command)  
    {
//        Tag::SMPP_SAR_MSG_REF_NUM

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
            break;
        case SUBMIT:
            sms = (SMS*)dta;
            break;
        case DELIVERY_RESP:
            sms = ((SmsResp*)dta)->get_sms();
            break;
        case SUBMIT_RESP:
            sms = ((SmsResp*)dta)->get_sms();
            break;
        default:
            throw SCAGException("Command Bridge Error: Unknown cmdid for SCAGCommand");
        }

        return sms->getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
    }
};


}}

#endif

