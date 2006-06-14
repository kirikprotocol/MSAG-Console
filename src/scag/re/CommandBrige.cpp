
#include "CommandBrige.h"
#include <scag/sessions/Session.h>
#include "util/recoder/recode_dll.h"
#include "scag/transport/http/HttpCommand.h"
#include "scag/bill/BillingManager.h"
#include "scag/sessions/Session.h"

#include "scag/util/encodings/Encodings.h"

namespace scag { namespace re {

using namespace scag::transport::http;
using namespace scag::bill;
using namespace scag::sessions;
using namespace scag::util::encodings;

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
whoisd-charging	Byte	0x4901
whoisd-message-transport-type	Byte	0x4902
whoisd-message-content-type	String	0x4903
whoisd-expected-message-transport-type	Byte	0x4904
whoisd-expected-message-content-type	String	0x4905

*/


EventHandlerType CommandBrige::getHTTPHandlerType(const SCAGCommand& command)
{
    switch(command.getCommandId())
    {
        case HTTP_REQUEST:
            return EH_HTTP_REQUEST;
        case HTTP_RESPONSE:
            return EH_HTTP_RESPONSE;
        case HTTP_DELIVERY:
            return EH_HTTP_DELIVERY;
        default:
            return EH_UNKNOWN;
    }
}

EventHandlerType CommandBrige::getSMPPHandlerType(const SCAGCommand& command)
{
    EventHandlerType handlerType;

    SCAGCommand& _command = const_cast<SCAGCommand&>(command);
    SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);

    if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");

    
    SMS& sms = getSMS(*smppCommand);
    CommandId cmdid = (*smppCommand)->get_commandId();

    CSmppDiscriptor SmppDiscriptor;
    int receiptMessageId = 0;

    switch (cmdid) 
    {
    case DELIVERY:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        if (receiptMessageId) handlerType = EH_RECEIPT;
        else handlerType = EH_DELIVER_SM;
        break;

    case SUBMIT:
        handlerType = EH_SUBMIT_SM;
        break;

    case DELIVERY_RESP:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        if (receiptMessageId) handlerType = EH_RECEIPT;
        else handlerType = EH_DELIVER_SM_RESP;
        break;

    case SUBMIT_RESP:
        handlerType = EH_SUBMIT_SM_RESP;
        break;

    default:
        handlerType = EH_UNKNOWN;
    }

    return handlerType;
}


EventHandlerType CommandBrige::getHandlerType(const SCAGCommand& command)
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



std::string CommandBrige::getMessageBody(SmppCommand& command)
{
    unsigned len = 0;

    const char * buff = 0;

    std::string str;
    char ucs2buff[2048];
    /*
        static const uint8_t SMSC7BIT             = 0;
        static const uint8_t LATIN1               = 3;
        static const uint8_t BINARY               = BIT(2);
        static const uint8_t UCS2                 = BIT(3);
    */
    SMS& data = getSMS(command);

    if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
    {
        buff = data.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    } else if (data.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD)) 
    {
        buff = data.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    } 

    if (buff == 0) return str;

    int code = smsc::smpp::DataCoding::SMSC7BIT;

    if (data.hasIntProperty(Tag::SMPP_DATA_CODING)) 
        code = data.getIntProperty(Tag::SMPP_DATA_CODING);

    switch (code) 
    {
    case smsc::smpp::DataCoding::SMSC7BIT:
        Convertor::GSM7BitToUTF8(buff,len,str);
        break;
    case smsc::smpp::DataCoding::LATIN1:
        Convertor::KOI8RToUTF8(buff, len, str);
        break;
    case smsc::smpp::DataCoding::UCS2:
        Convertor::UCS2ToUTF8((unsigned short *)buff, len / 2, str);
        break;
    default:
        Convertor::GSM7BitToUTF8(buff,len,str);
    }

    return str;
}



CSmppDiscriptor CommandBrige::getSmppDiscriptor(const SCAGCommand& command)
{

    SCAGCommand& _command = const_cast<SCAGCommand&>(command);

    SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
    if (!smppCommand) throw SCAGException("Command Bridge Error: Cannot get SmppDiscriptor - SCAGCommand is not smpp-type");


    SMS& sms = getSMS(*smppCommand);

    CommandId cmdid = (*smppCommand)->get_commandId();

    CSmppDiscriptor SmppDiscriptor;
    int receiptMessageId = 0;

    bool transact = false;
    bool req_receipt = false;

    if (sms.hasIntProperty(Tag::SMPP_ESM_CLASS))
    {
        //узнаём транзакционная или нет доставка
        int esm_class = sms.getIntProperty(Tag::SMPP_ESM_CLASS);
        transact = ((esm_class&2) == 2);
    }

    if (sms.hasIntProperty(Tag::SMPP_REGISTRED_DELIVERY))
    {
        //узнаём заказал ли сервис отчёт о доставке
        int reg_delivery = sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY);
        req_receipt = ((reg_delivery&3) > 0);
    } 

    SmppDiscriptor.m_waitReceipt = ((!transact)&&(req_receipt));
    
    switch (cmdid) 
    {
    case DELIVERY:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        //TODO: ensure
        if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT;
        else if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_DIALOG;
            //SmppDiscriptor.isUSSDClosed = (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST);
            /*SmppDiscriptor.bWantToOpen = (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_INDICATION)
                                             || (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP));*/

            if (smsc::smpp::UssdServiceOpValue::PSSR_INDICATION == sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP))
                  SmppDiscriptor.wantOpenUSSD = true;
        }
        else SmppDiscriptor.cmdType = CO_DELIVER;


        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
    case SUBMIT:

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_DIALOG;
            //TODO: снять ремарку SmppDiscriptor.wantOpenUSSD = (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == smsc::smpp::UssdServiceOpValue::USSR_REQUEST);
        }
        else SmppDiscriptor.cmdType = CO_SUBMIT;

        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
    case DELIVERY_RESP:

        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
        SmppDiscriptor.isResp = true;

        if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT;
        else if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_DIALOG;
            SmppDiscriptor.isUSSDClosed = ((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_RESPONSE)||((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST)));
        }
        else SmppDiscriptor.cmdType = CO_DELIVER;

        break;
    case SUBMIT_RESP:
        SmppDiscriptor.isResp = true;

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_DIALOG;
            SmppDiscriptor.isUSSDClosed = ((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_RESPONSE)||((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST)));
        }
        else SmppDiscriptor.cmdType = CO_SUBMIT;

        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
        break;
    }
        /*
        if (sms == 0) throw SCAGException("Command Bridge Error: Unknown SCAGCommand data");

        SmppDiscriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
          */

    return SmppDiscriptor;
}


int CommandBrige::getProtocolForEvent(SCAGCommand& command)
{
    TransportType cmdType = command.getType();

    if (cmdType == SMPP) 
    {
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&command);
        SMS& sms = getSMS(*smppCommand);

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) return PROTOCOL_SMPP_USSD;
        return PROTOCOL_SMPP_SMS;
    } 
    else if (cmdType == HTTP) return PROTOCOL_HTTP;
    else if (cmdType == MMS) return PROTOCOL_MMS;

    throw SCAGException("CommandBrige: Unknown command protocol");
}                 



}}


