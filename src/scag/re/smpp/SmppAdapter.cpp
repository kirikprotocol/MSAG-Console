#include "SmppAdapter.h"
#include <iostream>


namespace scag { namespace re { namespace smpp 
{

using namespace scag::util::properties;


Hash<int> SmppCommandAdapter::FieldNames = SmppCommandAdapter::InitFieldNames();



Hash<int> SmppCommandAdapter::InitFieldNames()
{

    Hash<int> hs;

    hs["ADDRESS"]               = ADDRESS;
    hs["DIALOG_ID"]             = DIALOG_ID;
    hs["MODE"]                  = MODE;
    hs["PRIORITY"]              = PRIORITY;
    hs["SME_INDEX"]             = SME_INDEX;
    hs["STATUS"]                = STATUS;

    hs["ATTEMPTS_COUNT"]        = ATTEMPTS_COUNT;
    hs["BILLING_RECORD"]        = BILLING_RECORD;
    hs["CONCAT_MSG_REF"]        = CONCAT_MSG_REF;
    hs["CONCAT_SEQ_NUM"]        = CONCAT_SEQ_NUM;
    hs["DEALIASED_DEST_ADDR"]   = DEALIASED_DEST_ADDR;
    hs["DELIVERY_REPORT"]       = DELIVERY_REPORT;
    hs["DEST_ADDR"]             = DEST_ADDR;
    hs["DEST_SME_ID"]           = DEST_SME_ID;
    hs["ESERVICE_TYPE"]         = ESERVICE_TYPE;
    hs["LAST_RESULT"]           = LAST_RESULT;
    hs["LAST_TIME"]             = LAST_TIME;
    hs["MSG_REFERENCE"]         = MSG_REFERENCE;
    hs["NEXT_TIME"]             = NEXT_TIME;
    hs["ORIGIN_ADDR"]           = ORIGIN_ADDR;
    hs["ROUTE_ID"]              = ROUTE_ID;
    hs["SERVICE_ID"]            = SERVICE_ID;
    hs["SOURCE_SME_ID"]         = SOURCE_SME_ID;
    hs["STATE"]                 = STATE;
    hs["SUBMIT_TIME"]           = SUBMIT_TIME;
    hs["VALID_TIME"]            = VALID_TIME;
    hs["ARCHIV_REQUEST"]        = ARCHIV_REQUEST;


    hs["NUM_DESTS"]             = NUM_DESTS;
    
    hs["DATA_SM"]               = DATA_SM;
    hs["MSG_ID"]                = MSG_ID;
    
    hs["SCHEDULE_DELIVERY_TIME"]= SCHEDULE_DELIVERY_TIME;
    hs["DEFAULT_MSG_ID"]        = DEFAULT_MSG_ID;
    hs["SM_LEN"]                = SM_LEN;
    hs["SOURCE_ADDR"]           = SOURCE_ADDR;
    hs["VALID_PERIOD"]          = VALID_PERIOD;
    
    hs["MSG_STATE"]             = MSG_STATE;
    hs["NETWORK_CODE"]          = NETWORK_CODE;
    
    
    hs["FORCE"]                 = FORCE;
    hs["INTERNAL"]              = INTERNAL;
    hs["SERVICE_TYPE"]          = SERVICE_TYPE;
    
    
    hs["PASS"]                  = PASS;
    hs["SYS_ID"]                = SYS_ID;

    return hs;
}


void SmppCommandAdapter::changed(AdapterProperty& property)
{
    if (!FieldNames.Exists(property.GetName().c_str())) 
    {
        return;
    }

    int FieldId = FieldNames.Get(property.GetName().c_str());


    _SmppCommand * cmd = command.operator ->();
    if (!cmd) return;

    CommandId cmdid = cmd->get_commandId();
    //Only for SMS
    SMS * data = (SMS *) cmd->dta;
    Address adr(property.getStr().c_str());

    if ((cmdid == DELIVERY)||(SUBMIT)) 
    {
        switch (FieldId) 
        {
        case ATTEMPTS_COUNT:
            data->setAttemptsCount(property.getInt());
            break;
        case BILLING_RECORD:
            data->setBillingRecord(property.getInt());
            break;

        case CONCAT_MSG_REF:
            data->setConcatMsgRef(property.getInt());
            break;

        case CONCAT_SEQ_NUM:
            data->setConcatSeqNum(property.getInt());
            break;

        case DEALIASED_DEST_ADDR:
            data->setDealiasedDestinationAddress(adr);
            break;

        case DELIVERY_REPORT:
            data->setDeliveryReport(property.getInt());
            break;

        case DEST_ADDR:
            data->setDestinationAddress(adr);
            break;

        case DEST_SME_ID:
            data->setDestinationSmeId(property.getStr().c_str());
            break;

        case ESERVICE_TYPE:
            data->setEServiceType(property.getStr().c_str());
            break;

        case LAST_RESULT:
            data->setLastResult(property.getInt());
            break;

        case LAST_TIME:
            data->lastTime = property.getInt();
            break;

        case MSG_REFERENCE:
            data->setMessageReference(property.getInt());
            break;

        case NEXT_TIME:
            data->setNextTime(property.getInt());
            break;

        case ORIGIN_ADDR:
            data->setOriginatingAddress(adr);
            break;

        case ROUTE_ID:
            data->setRouteId(property.getStr().c_str());
            break;

        case SERVICE_ID:
            data->setServiceId(property.getInt());
            break;

        case SOURCE_SME_ID:
            data->setSourceSmeId(property.getStr().c_str());
            break;

        case STATE:
            data->state = (State)property.getInt();
            break;

        case SUBMIT_TIME:
            data->setSubmitTime(property.getInt());
            break;

        case VALID_TIME:
            data->setValidTime(property.getInt());
            break;

        case ARCHIV_REQUEST:
            data->setArchivationRequested(property.getInt());
            break;
        default:
            return;
            break;
        }

        if (PropertyPul.Exist(FieldId))
        {
            PropertyPul.Delete(FieldId);
            /*AdapterProperty * p = PropertyPul.Get(FieldId);
            p->setStr(property.getStr());*/
        }
    }


}


Property* SmppCommandAdapter::getProperty(const std::string& name)
{
    _SmppCommand * cmd = command.operator ->();
    if (!cmd) return 0;

    CommandId cmdid = cmd->get_commandId();
    void * dta = cmd->dta;

    if (!dta) return 0;
    if (!FieldNames.Exists(name.c_str())) return 0;

    int FieldId;
    FieldId = FieldNames[name.c_str()];

    AdapterProperty * property = 0;


    switch (FieldId) 
    {
    case ADDRESS:
        return getPropertyFromPul(FieldId,name,cmd->get_address().toString());
    case DIALOG_ID:
        return getPropertyFromPul(FieldId,name,cmd->get_dialogId());
    case MODE:
        return getPropertyFromPul(FieldId,name,cmd->get_mode());
    case PRIORITY:
        return getPropertyFromPul(FieldId,name,cmd->get_priority());
    case SME_INDEX:
        return getPropertyFromPul(FieldId,name,cmd->get_smeIndex());
    case STATUS:
        return getPropertyFromPul(FieldId,name,cmd->get_status());
    default:
        switch ( cmdid )
        {
        case DELIVERY:
        case SUBMIT:
          return getProperty(*(SMS*)dta,name,FieldId);
        case SUBMIT_MULTI_SM:
          return getProperty(*(SubmitMultiSm*)dta,name,FieldId);
        case SUBMIT_MULTI_SM_RESP:
          return getProperty(*(SubmitMultiResp*)dta,name,FieldId);
        case DELIVERY_RESP:
        case SUBMIT_RESP:
          return getProperty(*(SmsResp*)dta,name,FieldId);
        case REPLACE:
          return getProperty(*(ReplaceSm*)dta,name,FieldId);
        case QUERY:
          return getProperty(*(QuerySm*)dta,name,FieldId);
        case QUERY_RESP:
          return getProperty(*(QuerySmResp*)dta,name,FieldId);
        case CANCEL:
          return getProperty(*(CancelSm*)dta,name,FieldId);
        case BIND_TRANSCEIVER:
          return getProperty(*(BindCommand*)dta,name,FieldId);
        }
        break;
    }
    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const SMS& data,const std::string& name,int FieldId)
{

    switch (FieldId) 
    {
    case ATTEMPTS_COUNT:
        return getPropertyFromPul(FieldId,name,data.getAttemptsCount());
    case BILLING_RECORD:
        return getPropertyFromPul(FieldId,name,data.getBillingRecord());
    case CONCAT_MSG_REF:
        return getPropertyFromPul(FieldId,name,data.getConcatMsgRef());
    case CONCAT_SEQ_NUM:
        return getPropertyFromPul(FieldId,name,data.getConcatSeqNum());
    case DEALIASED_DEST_ADDR:
        return getPropertyFromPul(FieldId,name,data.getDealiasedDestinationAddress().toString());
    case DELIVERY_REPORT:
        return getPropertyFromPul(FieldId,name,data.getDeliveryReport());
    case DEST_ADDR:
        return getPropertyFromPul(FieldId,name,data.getDestinationAddress().toString());
    case DEST_SME_ID:
        return getPropertyFromPul(FieldId,name,data.getDestinationSmeId());
    case ESERVICE_TYPE:
        return getPropertyFromPul(FieldId,name,data.getEServiceType());
    case LAST_RESULT:
        return getPropertyFromPul(FieldId,name,data.getLastResult());
    case LAST_TIME:
        return getPropertyFromPul(FieldId,name,data.getLastTime());
    case MSG_REFERENCE:
        return getPropertyFromPul(FieldId,name,data.getMessageReference());
    case NEXT_TIME:
        return getPropertyFromPul(FieldId,name,data.getNextTime());
    case ORIGIN_ADDR:
        return getPropertyFromPul(FieldId,name,data.getOriginatingAddress().toString());
    case ROUTE_ID:
        return getPropertyFromPul(FieldId,name,data.getRouteId());
    case SERVICE_ID:
        return getPropertyFromPul(FieldId,name,data.getServiceId());
    case SOURCE_SME_ID:
        return getPropertyFromPul(FieldId,name,data.getSourceSmeId());
    case STATE:
        return getPropertyFromPul(FieldId,name,data.getState());
    case SUBMIT_TIME:
        return getPropertyFromPul(FieldId,name,data.getSubmitTime());
    case VALID_TIME:
        return getPropertyFromPul(FieldId,name,data.getValidTime());
    case ARCHIV_REQUEST:
        return getPropertyFromPul(FieldId,name,data.isArchivationRequested());
    }
    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const SubmitMultiSm& data,const std::string& name,int FieldId)
{
    if (FieldId == NUM_DESTS)
    {
        return getPropertyFromPul(FieldId,name,data.number_of_dests);
    }
    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const SubmitMultiResp& data,const std::string& name,int FieldId)
{
    if (FieldId == NUM_DESTS)
    {
        return getPropertyFromPul(FieldId,name,data.get_messageId());
    }
    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const SmsResp& data,const std::string& name,int FieldId)
{
    if (FieldId == DATA_SM) 
    {
        return getPropertyFromPul(FieldId,name,data.get_dataSm());
    } else if (FieldId == MSG_ID) 
    {
        return getPropertyFromPul(FieldId,name,data.get_messageId());
    } else 
    {
        if (!data.get_sms()) return 0;
        return getProperty(*data.get_sms(),name,FieldId); 
    }
    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const ReplaceSm& data,const std::string& name,int FieldId)
{

    switch (FieldId) 
    {
    case MSG_ID:
        return getPropertyFromPul(FieldId,name,data.getMessageId());
    case SCHEDULE_DELIVERY_TIME:
        return getPropertyFromPul(FieldId,name,data.scheduleDeliveryTime);
    case DEFAULT_MSG_ID:
        return getPropertyFromPul(FieldId,name,data.smDefaultMsgId);
    case SM_LEN:
        return getPropertyFromPul(FieldId,name,data.smLength);
    case SOURCE_ADDR:
        return getPropertyFromPul(FieldId,name,data.sourceAddr.get());
    case VALID_PERIOD:
        return getPropertyFromPul(FieldId,name,data.validityPeriod);
    }
    return 0;
}


AdapterProperty * SmppCommandAdapter::getProperty(const QuerySm& data,const std::string& name,int FieldId)
{

    if (FieldId == MSG_ID) 
    {
        return getPropertyFromPul(FieldId,name,data.getMessageId());
    } else if (FieldId== SOURCE_ADDR) 
    {
        return getPropertyFromPul(FieldId,name,data.sourceAddr.get());
    }

    return 0;
}


AdapterProperty * SmppCommandAdapter::getProperty(const QuerySmResp& data,const std::string& name,int FieldId)
{

    switch (FieldId) 
    {
    case MSG_ID:
        return getPropertyFromPul(FieldId,name,data.messageId.get());
    case STATUS:
        return getPropertyFromPul(FieldId,name,data.commandStatus);
    case MSG_STATE:
        return getPropertyFromPul(FieldId,name,data.messageState);
    case NETWORK_CODE:
        return getPropertyFromPul(FieldId,name,data.networkCode);
    }

    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const CancelSm& data,const std::string& name,int FieldId)
{
    switch(FieldId)
    {
    case MSG_ID:
        return getPropertyFromPul(FieldId,name,data.getMessageId());
    case SOURCE_ADDR:
        return getPropertyFromPul(FieldId,name,data.sourceAddr.get());
    case DEST_ADDR:
        return getPropertyFromPul(FieldId,name,data.destAddr.get());
    case FORCE:
        return getPropertyFromPul(FieldId,name,data.force);
    case INTERNAL:
        return getPropertyFromPul(FieldId,name,data.internall);
    case SERVICE_TYPE:
        return getPropertyFromPul(FieldId,name,data.serviceType.get());
    } 

    return 0;
}

AdapterProperty * SmppCommandAdapter::getProperty(const BindCommand& data,const std::string& name,int FieldId)
{
    if (FieldId == PASS) 
    {
        return getPropertyFromPul(FieldId,name,data.pass);
    } else if (FieldId == SYS_ID) 
    {
        return getPropertyFromPul(FieldId,name,data.sysId);
    }
    return 0;
}



AdapterProperty * SmppCommandAdapter::getPropertyFromPul(int FieldId,const std::string& InitName,const std::string& InitValue)
{
    if (!PropertyPul.Exist(FieldId)) 
    {
        AdapterProperty * property = new AdapterProperty(InitName,this,InitValue);
        PropertyPul.Insert(FieldId,property);
    }
    return PropertyPul.Get(FieldId);

}

AdapterProperty * SmppCommandAdapter::getPropertyFromPul(int FieldId,const std::string& InitName,const int InitValue)
{
    if (!PropertyPul.Exist(FieldId)) 
    {
        AdapterProperty * property = new AdapterProperty(InitName,this,InitValue);
        PropertyPul.Insert(FieldId,property);
    }
    return PropertyPul.Get(FieldId);
}


SmppCommandAdapter::~SmppCommandAdapter()
{
    int key = 0;
    AdapterProperty * value = 0;

    for (IntHash <AdapterProperty *>::Iterator it = PropertyPul.First(); it.Next(key, value);)
    {
        delete value;
    }
}




}}}
