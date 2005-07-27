#include "SmppAdapter.h"
#include <iostream>


namespace scag { namespace re { namespace smpp 
{
using namespace std;

using namespace scag::util::properties;




void SmppCommandAdapter::changed(const NamedProperty& property)
{

}


NamedProperty* SmppCommandAdapter::getProperty(const std::string& name)
{

    _SmppCommand * cmd = command.operator ->();
    if (!cmd) return 0;

    CommandId cmdid = cmd->get_commandId();
    void * dta = cmd->dta;

    if (!dta) return 0;
    
    NamedProperty * property = 0;


    //cmd->get_bindCommand();

    if (name=="ADDRESS") 
    {
        property = getPropertyFromPul(name,cmd->get_address().toString());
    } else if (name=="DIALOG_ID") 
    {
        property = getPropertyFromPul(name,cmd->get_dialogId());
    } else if (name=="MODE")
    {
        property = getPropertyFromPul(name,cmd->get_mode());
    } else if (name=="PRIORITY") 
    {
        property = getPropertyFromPul(name,cmd->get_priority());
    } else if (name=="SME_INDEX") 
    {
        property = getPropertyFromPul(name,cmd->get_smeIndex());
    } else if (name=="STATUS") 
    {
        property = getPropertyFromPul(name,cmd->get_status());
    } else

    switch ( cmdid )
    {
    case DELIVERY:
    case SUBMIT:
      property = getProperty(*(SMS*)dta,name);
      break;

    case SUBMIT_MULTI_SM:
      property = getProperty(*(SubmitMultiSm*)dta,name);
      
      break;
    case SUBMIT_MULTI_SM_RESP:
      property = getProperty(*(SubmitMultiResp*)dta,name);
      break;

    case DELIVERY_RESP:
    case SUBMIT_RESP:
      property = getProperty(*(SmsResp*)dta,name);
      break;

    case REPLACE:
      property = getProperty(*(ReplaceSm*)dta,name);
      break;

    case QUERY:
      property = getProperty(*(QuerySm*)dta,name);
      break;
    case QUERY_RESP:
      property = getProperty(*(QuerySmResp*)dta,name);
      break;
    case CANCEL:
      property = getProperty(*(CancelSm*)dta,name);
      break;
    case BIND_TRANSCEIVER:
      property = getProperty(*(BindCommand*)dta,name);
      break;     
    default:
        return 0;
        break;
    }
    return property;
}
  

NamedProperty * SmppCommandAdapter::getProperty(const SMS& data,const std::string& name)
{
    if (name=="ATTEMPTS_COUNT") 
    {
        return getPropertyFromPul(name,data.getAttemptsCount());
    } else if (name=="BILLING_RECORD") 
    {
        return getPropertyFromPul(name,data.getBillingRecord());
    } /*else if (name=="BIN_PROPERTY") 
    {
        result = new NamedProperty("",0);
        data.getBinProperty();
    }*/ else if (name=="CONCAT_MSG_REF") 
    {
        return getPropertyFromPul(name,data.getConcatMsgRef());
    } else if (name=="CONCAT_SEQ_NUM") 
    {
        return getPropertyFromPul(name,data.getConcatSeqNum());
    } else if (name=="DEALIASED_DEST_ADDR") 
    {
        return getPropertyFromPul(name,data.getDealiasedDestinationAddress().toString());
    } else if (name=="DELIVERY_REPORT") 
    {
        return getPropertyFromPul(name,data.getDeliveryReport());
    } else if (name=="DEST_ADDR") 
    {
        return getPropertyFromPul(name,data.getDestinationAddress().toString());
    } /*else if (name=="DEST_DESCRIPTOR") 
    {
        result = new NamedProperty("",0);
        data.getDestinationDescriptor();
    }*/ else if (name=="DEST_SME_ID") 
    {
        return getPropertyFromPul(name,data.getDestinationSmeId());
    } else if (name=="ESERVICE_TYPE") 
    {
        return getPropertyFromPul(name,data.getEServiceType());
    } else if (name=="LAST_RESULT") 
    {
        return getPropertyFromPul(name,data.getLastResult());
    } else if (name=="LAST_TIME") 
    {
        return getPropertyFromPul(name,data.getLastTime());
    } else if (name=="MSG_REFERENCE") 
    {
        return getPropertyFromPul(name,data.getMessageReference());
    } else if (name=="NEXT_TIME") 
    {
        return getPropertyFromPul(name,data.getNextTime());
    } else if (name=="ORIGIN_ADDR") 
    {
        return getPropertyFromPul(name,data.getOriginatingAddress().toString());
    } else if (name=="ATTEMPTS_COUNT") 
    {
        return getPropertyFromPul(name,data.getPriority());
    } else if (name=="ROUTE_ID") 
    {
        return getPropertyFromPul(name,data.getRouteId());
    } else if (name=="SERVICE_ID") 
    {
        return getPropertyFromPul(name,data.getServiceId());
    } else if (name=="SOURCE_SME_ID") 
    {
        return getPropertyFromPul(name,data.getSourceSmeId());
    } else if (name=="STATE") 
    {
        return getPropertyFromPul(name,data.getState());
    } else if (name=="SUBMIT_TIME") 
    {
        return getPropertyFromPul(name,data.getSubmitTime());
    } else if (name=="VALID_TIME") 
    {
        return getPropertyFromPul(name,data.getValidTime());
    } else if (name=="ARCHIV_REQUEST") 
    {
        return getPropertyFromPul(name,data.isArchivationRequested());
    } 

    //data.getIntProperty() ??
    //data.getStringField(
    //data.getStrProperty( ??
    //data.getOriginatingDescriptor()
    //data.getMessageBody(
    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const SubmitMultiSm& data,const std::string& name)
{
    if (name == "NUM_DESTS")
    {
        return getPropertyFromPul(name,data.number_of_dests);
    }
    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const SubmitMultiResp& data,const std::string& name)
{
    if (name == "NUM_DESTS")
    {
        return getPropertyFromPul(name,data.get_messageId());
    }
    //data.get_status() ??
    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const SmsResp& data,const std::string& name)
{
    if (name=="DATA_SM") 
    {
        return getPropertyFromPul(name,data.get_dataSm());
    } else if (name=="MSG_ID") 
    {
        return getPropertyFromPul(name,data.get_messageId());
    } else 
    {
        if (!data.get_sms()) return 0;
        return getProperty(*data.get_sms(),name); 
    }
    //data.get_status() //?

    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const ReplaceSm& data,const std::string& name)
{
    if (name=="MSG_ID") 
    {
        return getPropertyFromPul(name,data.getMessageId());
    } else if (name=="SCHEDULE_DELIVERY_TIME") 
    {
        return getPropertyFromPul(name,data.scheduleDeliveryTime);
    } else if (name=="DEFAULT_MSG_ID") 
    {
        return getPropertyFromPul(name,data.smDefaultMsgId);
    } else if (name=="SM_LEN") 
    {
        return getPropertyFromPul(name,data.smLength);
    } else if (name=="SOURCE_ADDR") 
    {
        return getPropertyFromPul(name,data.sourceAddr.get());
    } else if (name=="VALID_PERIOD") 
    {
        return getPropertyFromPul(name,data.validityPeriod);
    }

    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const QuerySm& data,const std::string& name)
{

    if (name=="MSG_ID") 
    {
        return getPropertyFromPul(name,data.getMessageId());
    } else if (name=="SOURCE_ADDR") 
    {
        return getPropertyFromPul(name,data.sourceAddr.get());
    }

    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const QuerySmResp& data,const std::string& name)
{

    if (name=="MSG_ID") 
    {
        return getPropertyFromPul(name,data.messageId.get());
    } else if (name=="STATUS") 
    {
        return getPropertyFromPul(name,data.commandStatus);
    } else if (name=="MSG_STATE") 
    {
        return getPropertyFromPul(name,data.messageState);
    } else if (name=="NETWORK_CODE") 
    {
        return getPropertyFromPul(name,data.networkCode);
    }

    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const CancelSm& data,const std::string& name)
{
    NamedProperty * result = 0;

    if (name=="MSG_ID") 
    {
        return getPropertyFromPul(name,data.getMessageId());
    } else if (name=="SOURCE_ADDR") 
    {
        return getPropertyFromPul(name,data.sourceAddr.get());
    } else if (name=="DEST_ADDR") 
    {
        return getPropertyFromPul(name,data.destAddr.get());
    } else if (name=="FORCE") 
    {
        return getPropertyFromPul(name,data.force);
    } else if (name=="INTERNAL") 
    {
        return getPropertyFromPul(name,data.internall);
    } else if (name=="SERVICE_TYPE") 
    {
        return getPropertyFromPul(name,data.serviceType.get());
    } 

    return 0;
}

NamedProperty * SmppCommandAdapter::getProperty(const BindCommand& data,const std::string& name)
{

    if (name=="PASS") 
    {
        return getPropertyFromPul(name,data.pass);
    } else if (name=="SYS_ID") 
    {
        return getPropertyFromPul(name,data.sysId);
    }

    return 0;
}



NamedProperty * SmppCommandAdapter::getPropertyFromPul(const std::string& name,const std::string& InitValue)
{
    if (!PropertyPul.Exists(name.c_str())) 
    {
        NamedProperty * property = new NamedProperty(name,0);
        property->setStr(InitValue);
        PropertyPul.Insert(name.c_str(),property);
    }
    return PropertyPul.Get(name.c_str());

}

NamedProperty * SmppCommandAdapter::getPropertyFromPul(const std::string& name,const int InitValue)
{
    if (!PropertyPul.Exists(name.c_str())) 
    {
        NamedProperty * property = new NamedProperty(name,0);
        property->setInt(InitValue);
        PropertyPul.Insert(name.c_str(),property);
    }
    return PropertyPul.Get(name.c_str());
}


SmppCommandAdapter::~SmppCommandAdapter()
{
    char * key = 0;
    NamedProperty * value = 0;

    PropertyPul.First();
    while (PropertyPul.Next(key,value))
    {
        if (value) delete value;
        cout << "Property released" << endl;
    }
}




}}}
