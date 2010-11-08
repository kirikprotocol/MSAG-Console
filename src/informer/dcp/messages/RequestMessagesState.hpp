#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_REQUESTMESSAGESSTATE_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_REQUESTMESSAGESSTATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryMessageState.hpp"
#include "ReqField.hpp"


#ident "@(#) RequestMessagesState version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class RequestMessagesState{
public:
  RequestMessagesState()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    deliveryIdFlag=false;
    statesFlag=false;
    states.clear();
    msisdnFilterFlag=false;
    msisdnFilter.clear();
    startDateFlag=false;
    endDateFlag=false;
    fieldsFlag=false;
    fields.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 43;
  }

  static std::string messageGetName()
  {
    return "RequestMessagesState";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(deliveryIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryId=";
      sprintf(buf,"%d",deliveryId);
      rv+=buf;
    }
    if(statesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="states=";
      rv+="[";
      bool first=true;
      for(std::vector<DeliveryMessageState>::const_iterator it=states.begin(),end=states.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=DeliveryMessageState::getNameByValue(*it);
      }
    }
    if(msisdnFilterFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="msisdnFilter=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=msisdnFilter.begin(),end=msisdnFilter.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    if(startDateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="startDate=";
      rv+=startDate;
    }
    if(endDateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="endDate=";
      rv+=endDate;
    }
    if(fieldsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="fields=";
      rv+="[";
      bool first=true;
      for(std::vector<ReqField>::const_iterator it=fields.begin(),end=fields.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=ReqField::getNameByValue(*it);
      }
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(deliveryIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveryId);
    }
    if(statesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(states);
 
    }
    if(msisdnFilterFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(msisdnFilter);
    }
    if(startDateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(startDate);
    }
    if(endDateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(endDate);
    }
    if(fieldsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(fields);
 
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getDeliveryId()const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveryId");
    }
    return deliveryId;
  }
  void setDeliveryId(int32_t argValue)
  {
    deliveryId=argValue;
    deliveryIdFlag=true;
  }
  int32_t& getDeliveryIdRef()
  {
    deliveryIdFlag=true;
    return deliveryId;
  }
  bool hasDeliveryId()const
  {
    return deliveryIdFlag;
  }
  const std::vector<DeliveryMessageState>& getStates()const
  {
    if(!statesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("states");
    }
    return states;
  }
  void setStates(const std::vector<DeliveryMessageState>& argValue)
  {
    states=argValue;
    statesFlag=true;
  }
  std::vector<DeliveryMessageState>& getStatesRef()
  {
    statesFlag=true;
    return states;
  }
  bool hasStates()const
  {
    return statesFlag;
  }
  const std::vector<std::string>& getMsisdnFilter()const
  {
    if(!msisdnFilterFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("msisdnFilter");
    }
    return msisdnFilter;
  }
  void setMsisdnFilter(const std::vector<std::string>& argValue)
  {
    msisdnFilter=argValue;
    msisdnFilterFlag=true;
  }
  std::vector<std::string>& getMsisdnFilterRef()
  {
    msisdnFilterFlag=true;
    return msisdnFilter;
  }
  bool hasMsisdnFilter()const
  {
    return msisdnFilterFlag;
  }
  const std::string& getStartDate()const
  {
    if(!startDateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("startDate");
    }
    return startDate;
  }
  void setStartDate(const std::string& argValue)
  {
    startDate=argValue;
    startDateFlag=true;
  }
  std::string& getStartDateRef()
  {
    startDateFlag=true;
    return startDate;
  }
  bool hasStartDate()const
  {
    return startDateFlag;
  }
  const std::string& getEndDate()const
  {
    if(!endDateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("endDate");
    }
    return endDate;
  }
  void setEndDate(const std::string& argValue)
  {
    endDate=argValue;
    endDateFlag=true;
  }
  std::string& getEndDateRef()
  {
    endDateFlag=true;
    return endDate;
  }
  bool hasEndDate()const
  {
    return endDateFlag;
  }
  const std::vector<ReqField>& getFields()const
  {
    if(!fieldsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("fields");
    }
    return fields;
  }
  void setFields(const std::vector<ReqField>& argValue)
  {
    fields=argValue;
    fieldsFlag=true;
  }
  std::vector<ReqField>& getFieldsRef()
  {
    fieldsFlag=true;
    return fields;
  }
  bool hasFields()const
  {
    return fieldsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!startDateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("startDate");
    }
    if(!endDateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("endDate");
    }
    if(!fieldsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("fields");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(deliveryIdTag);
    ds.writeInt32LV(deliveryId); 
    ds.writeTag(startDateTag);
    ds.writeStrLV(startDate); 
    ds.writeTag(endDateTag);
    ds.writeStrLV(endDate); 
    ds.writeTag(fieldsTag);
    ds.writeLength(DataStream::fieldSize(fields));
    for(std::vector<ReqField>::const_iterator it=fields.begin(),end=fields.end();it!=end;++it)
    {
      ds.writeByte(it->getValue());
    }
 
    if(statesFlag)
    {
      ds.writeTag(statesTag);
    ds.writeLength(DataStream::fieldSize(states));
    for(std::vector<DeliveryMessageState>::const_iterator it=states.begin(),end=states.end();it!=end;++it)
    {
      ds.writeByte(it->getValue());
    }
 
    }
    if(msisdnFilterFlag)
    {
      ds.writeTag(msisdnFilterTag);
    ds.writeLength(DataStream::fieldSize(msisdnFilter));
    for(std::vector<std::string>::const_iterator it=msisdnFilter.begin(),end=msisdnFilter.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    }
    ds.writeTag(DataStream::endOfMessage_tag);
  }

  template <class DataStream>
  void deserialize(DataStream& ds)
  {
    Clear();
    bool endOfMessage=false;
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("RequestMessagesState");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case deliveryIdTag:
        {
          if(deliveryIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveryId");
          }
          deliveryId=ds.readInt32LV();
          deliveryIdFlag=true;
        }break;
        case statesTag:
        {
          if(statesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("states");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            states.push_back(ds.readByte());
            rd+=DataStream::fieldSize(states.back());
          }
          statesFlag=true;
        }break;
        case msisdnFilterTag:
        {
          if(msisdnFilterFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("msisdnFilter");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            msisdnFilter.push_back(ds.readStr());
            rd+=DataStream::fieldSize(msisdnFilter.back());
            rd+=DataStream::lengthTypeSize;
          }
          msisdnFilterFlag=true;
        }break;
        case startDateTag:
        {
          if(startDateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("startDate");
          }
          startDate=ds.readStrLV();
          startDateFlag=true;
        }break;
        case endDateTag:
        {
          if(endDateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("endDate");
          }
          endDate=ds.readStrLV();
          endDateFlag=true;
        }break;
        case fieldsTag:
        {
          if(fieldsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("fields");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            fields.push_back(ds.readByte());
            rd+=DataStream::fieldSize(fields.back());
          }
          fieldsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("RequestMessagesState",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!deliveryIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("deliveryId");
    }
    if(!startDateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("startDate");
    }
    if(!endDateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("endDate");
    }
    if(!fieldsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("fields");
    }

  }

  int32_t messageGetSeqNum()const
  {
    return seqNum;
  }

  void messageSetSeqNum(int32_t argValue)
  {
    seqNum=argValue;
  }

  void messageSetConnId(int argConnId)
  {
    connId=argConnId;
  }

  int messageGetConnId()const
  {
    return connId;
  }
 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t deliveryIdTag=1;
  static const int32_t statesTag=2;
  static const int32_t msisdnFilterTag=7;
  static const int32_t startDateTag=4;
  static const int32_t endDateTag=5;
  static const int32_t fieldsTag=6;

  int32_t seqNum;
  int connId;

  int32_t deliveryId;
  std::vector<DeliveryMessageState> states;
  std::vector<std::string> msisdnFilter;
  std::string startDate;
  std::string endDate;
  std::vector<ReqField> fields;

  bool deliveryIdFlag;
  bool statesFlag;
  bool msisdnFilterFlag;
  bool startDateFlag;
  bool endDateFlag;
  bool fieldsFlag;
};

}
}
}
}
#endif
