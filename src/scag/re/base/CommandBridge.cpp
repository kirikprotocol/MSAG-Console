#include "CommandBridge.h"
// #include "scag/bill/base/BillingManager.h"
#include "scag/sessions/base/Session2.h"
// #include "scag/transport/http/HttpCommand.h"
#include "scag/util/encodings/Encodings.h"
#include "util/recoder/recode_dll.h"

namespace scag2 {
namespace re {

// using namespace scag::transport::http;
using namespace sessions;
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
        // FIXME: http
        /*
        case HTTP_REQUEST:
            return EH_HTTP_REQUEST;
        case HTTP_RESPONSE:
            return EH_HTTP_RESPONSE;
        case HTTP_DELIVERY:
            return EH_HTTP_DELIVERY;
         */
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

    
    SMS& sms = getSMS(*smppCommand);
    CommandId cmdid = CommandId(smppCommand->getCommandId());

//    CSmppDiscriptor SmppDiscriptor;
    int receiptMessageId = 0;

    switch (cmdid) 
    {
    case DELIVERY:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
        handlerType = receiptMessageId ? EH_RECEIPT : EH_DELIVER_SM;
        break;

    case SUBMIT:
        handlerType = EH_SUBMIT_SM;
        break;

    case DELIVERY_RESP:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
        handlerType = receiptMessageId ? EH_RECEIPT : EH_DELIVER_SM_RESP;
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
    unsigned len = 0;

    const char * buff = 0;

    std::string str;
    // char ucs2buff[2048];
    /*
        static const uint8_t SMSC7BIT             = 0;
        static const uint8_t LATIN1               = 3;
        static const uint8_t BINARY               = BIT(2);
        static const uint8_t UCS2                 = BIT(3);
    */
    SMS& data = getSMS(command);

    if (data.hasBinProperty(Tag::SMPP_SHORT_MESSAGE)) 
        buff = data.getBinProperty(Tag::SMPP_SHORT_MESSAGE, &len);
    if (!len && data.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD)) 
        buff = data.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);

    if (!buff || !len) return str;

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


#if 0
CSmppDescriptor CommandBridge::getSmppDescriptor(const SCAGCommand& command)
{

    SCAGCommand& _command = const_cast<SCAGCommand&>(command);

    SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
    if (!smppCommand) throw SCAGException("Command Bridge Error: Cannot get SmppDescriptor - SCAGCommand is not smpp-type");


    SMS& sms = getSMS(*smppCommand);

    CommandId cmdid = CommandId(smppCommand->getCommandId());

    CSmppDescriptor SmppDescriptor;
    int receiptMessageId = 0;
    SmsResp * smsResp = 0;

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

    SmppDescriptor.m_waitReceipt = ((!transact)&&(req_receipt));
    
    switch (cmdid) 
    {
    case DELIVERY:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        //TODO: ensure
        if (receiptMessageId) SmppDescriptor.cmdType = CO_RECEIPT;
        else if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDescriptor.cmdType = CO_USSD_DIALOG;
            SmppDescriptor.wantOpenUSSD = 
                (uint32_t(smsc::smpp::UssdServiceOpValue::PSSR_INDICATION) == sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
        }
        else SmppDescriptor.cmdType = CO_DELIVER;

        if (sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
            SmppDescriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

        if (sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
            SmppDescriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
    case SUBMIT:

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDescriptor.cmdType = CO_USSD_DIALOG;
            SmppDescriptor.wantOpenUSSD = smppCommand->flagSet(SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG);
        }
        else SmppDescriptor.cmdType = CO_SUBMIT;

        if (sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
            SmppDescriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

        if (sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
            SmppDescriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
    case DELIVERY_RESP:

        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        if (sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
            SmppDescriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

        if (sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
            SmppDescriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        SmppDescriptor.isResp = true;

        if (receiptMessageId) SmppDescriptor.cmdType = CO_RECEIPT;
        else if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDescriptor.cmdType = CO_USSD_DIALOG;
//            SmppDescriptor.isUSSDClosed = ((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_RESPONSE)||((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST)));
            SmppDescriptor.isUSSDClosed = 
                (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == uint32_t(USSN_CONFIRM));
        }
        else SmppDescriptor.cmdType = CO_DELIVER;

        break;
    case SUBMIT_RESP:
        SmppDescriptor.isResp = true;

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDescriptor.cmdType = CO_USSD_DIALOG;
            SmppDescriptor.isUSSDClosed = 
                (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == uint32_t(PSSR_RESPONSE));
        }
        else SmppDescriptor.cmdType = CO_SUBMIT;

        if (sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
            SmppDescriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

        if (sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
            SmppDescriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
        break;

    case DATASM:

        switch (smppCommand->get_smsCommand().dir)
        {
        case dsdSc2Sc:
            SmppDescriptor.cmdType = CO_DATA_SC_2_SC;
            break;
        case dsdSc2Srv:
            SmppDescriptor.cmdType = CO_DATA_SC_2_SME;
            break;
        case dsdSrv2Sc:
            SmppDescriptor.cmdType = CO_DATA_SME_2_SC;
            break;
        case dsdSrv2Srv:
            SmppDescriptor.cmdType = CO_DATA_SME_2_SME;
            break;
        case dsdUnknown:
        default:
            throw SCAGException("Command Bridge: cannot identify DATA_SM direction");
            break;

        }

        if (sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
            SmppDescriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

        if (sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
            SmppDescriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;

    case DATASM_RESP:
        smsResp = smppCommand->get_resp();
        if (!smsResp) throw SCAGException("Command Bridge: cannot get SmsCommand from DATA_SM");
        SmppDescriptor.isResp = true;

        switch (smsResp->get_dir()) 
        {
        case dsdSc2Sc:
            SmppDescriptor.cmdType = CO_DATA_SC_2_SC;
            break;
        case dsdSc2Srv:
            SmppDescriptor.cmdType = CO_DATA_SC_2_SME;
            break;
        case dsdSrv2Sc:
            SmppDescriptor.cmdType = CO_DATA_SME_2_SC;
            break;
        case dsdSrv2Srv:
            SmppDescriptor.cmdType = CO_DATA_SME_2_SME;
            break;
        case dsdUnknown:
        default:
            throw SCAGException("Command Bridge: cannot identify DATA_SM direction");
            break;

        }
        if (sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)) 
            SmppDescriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);

        if (sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)) 
            SmppDescriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
  /*  case PROCESSEXPIREDRESP:
        SmppDescriptor.isResp = true;
        SmppDescriptor.

        break;*/
    default:
        break;
    } // switch
        /*
        if (sms == 0) throw SCAGException("Command Bridge Error: Unknown SCAGCommand data");

        SmppDescriptor.lastIndex = sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDescriptor.currentIndex = sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
          */

    return SmppDescriptor;
}
#endif


int CommandBridge::getProtocolForEvent(SCAGCommand& command)
{
    TransportType cmdType = command.getType();

    if (cmdType == SMPP) 
    {
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&command);
        SMS& sms = getSMS(*smppCommand);

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) return PROTOCOL_SMPP_USSD;
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


void AssignAddress(Address& address, const char * str)
{
    Address addr(str);
    address = addr;
}


}}


