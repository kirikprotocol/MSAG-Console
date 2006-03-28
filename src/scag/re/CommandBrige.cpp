
#include "CommandBrige.h"
#include <scag/sessions/Session.h>
#include "util/recoder/recode_dll.h"

namespace scag { namespace re {

EventHandlerType CommandBrige::getHandlerType(const SCAGCommand& command)
{
    EventHandlerType handlerType = EH_UNKNOWN;

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



CSmppDiscriptor CommandBrige::getSmppDiscriptor(const SCAGCommand& command)
{

    SCAGCommand& _command = const_cast<SCAGCommand&>(command);

    SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
    if (!smppCommand) throw SCAGException("Command Bridge Error: Cannot get SmppDiscriptor - SCAGCommand is not smpp-type");


    SMS& sms = getSMS(*smppCommand);
    CommandId cmdid = (*smppCommand)->get_commandId();

    CSmppDiscriptor SmppDiscriptor;
    int receiptMessageId = 0;
    SmppDiscriptor.isUSSDClosed = false;
    SmppDiscriptor.wantOpenUSSD = false;
    
    switch (cmdid) 
    {
    case DELIVERY:
        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT_DELIVER_SM;
        else if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_DELIVER;
            //SmppDiscriptor.isUSSDClosed = (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST);
            /*SmppDiscriptor.bWantToOpen = (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_INDICATION)
                                             || (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP));*/

            if (smsc::smpp::UssdServiceOpValue::PSSR_INDICATION == sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP))
                  SmppDiscriptor.wantOpenUSSD = true;
        }
        else SmppDiscriptor.cmdType = CO_DELIVER_SM;


        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
    case SUBMIT:

        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_SUBMIT;
            //TODO: снять ремарку SmppDiscriptor.wantOpenUSSD = (sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == smsc::smpp::UssdServiceOpValue::USSR_REQUEST);
        }
        else SmppDiscriptor.cmdType = CO_SUBMIT_SM;

        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        break;
    case DELIVERY_RESP:

        receiptMessageId = atoi(sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());

        SmppDiscriptor.lastIndex = sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        SmppDiscriptor.currentIndex = sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);

        if (receiptMessageId) SmppDiscriptor.cmdType = CO_RECEIPT_DELIVER_SM_RESP;
        else if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_DELIVER_RESP;
            SmppDiscriptor.isUSSDClosed = ((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_RESPONSE)||((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST)));
        }
        else SmppDiscriptor.cmdType = CO_DELIVER_SM_RESP;

        break;
    case SUBMIT_RESP:
        if (sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) 
        {
            SmppDiscriptor.cmdType = CO_USSD_SUBMIT_RESP;
            SmppDiscriptor.isUSSDClosed = ((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == PSSR_RESPONSE)||((sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) == USSN_REQUEST)));
        }
        else SmppDiscriptor.cmdType = CO_SUBMIT_SM_RESP;

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



void CommandBrige::makeTrafficEvent(SmppCommand& command, int handlerType, scag::sessions::CSessionPrimaryKey& sessionPrimaryKey, SACC_TRAFFIC_INFO_EVENT_t& ev)
{
    ev.Header.cCommandId = handlerType;
        
    ev.Header.cProtocolId = 1;
    ev.Header.iServiceId = command.getServiceId();
    ev.Header.iServiceProviderId = 1;

    long now;
    time(&now);

    ev.Header.lDateTime = now;

    const char * str = getAbonentAddr(command).toString().c_str();
    sprintf((char *)ev.Header.pAbonentNumber,"%s",str);

    ev.Header.sCommandStatus = 1;
    ev.Header.sEventType = 1;

    ev.iOperatorId = 1;

    std::string unicodeSTR = getMessageBody(command);
    memcpy(ev.pMessageText, unicodeSTR.data(), unicodeSTR.size()); 

    sprintf((char *)ev.pSessionKey,"%s/%d", sessionPrimaryKey.abonentAddr.toString().c_str(),sessionPrimaryKey.BornMicrotime);
}


}}


