#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERIESLIST_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERIESLIST_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveriesFilter.hpp"
#include "DeliveryFields.hpp"


#ident "@(#) GetDeliveriesList version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetDeliveriesList{
public:
  GetDeliveriesList()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    filterFlag=false;
    resultFieldsFlag=false;
    resultFields.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 50;
  }

  static std::string messageGetName()
  {
    return "GetDeliveriesList";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(filterFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="filter=";
      rv+='(';
      rv+=filter.toString();
      rv+=')';
    }
    if(resultFieldsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="resultFields=";
      rv+="[";
      bool first=true;
      for(std::vector<DeliveryFields>::const_iterator it=resultFields.begin(),end=resultFields.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=DeliveryFields::getNameByValue(*it);
      }
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(filterFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=filter.length<DataStream>();
    }
    if(resultFieldsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(resultFields);
 
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const DeliveriesFilter& getFilter()const
  {
    if(!filterFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("filter");
    }
    return filter;
  }
  void setFilter(const DeliveriesFilter& argValue)
  {
    filter=argValue;
    filterFlag=true;
  }
  DeliveriesFilter& getFilterRef()
  {
    filterFlag=true;
    return filter;
  }
  bool hasFilter()const
  {
    return filterFlag;
  }
  const std::vector<DeliveryFields>& getResultFields()const
  {
    if(!resultFieldsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("resultFields");
    }
    return resultFields;
  }
  void setResultFields(const std::vector<DeliveryFields>& argValue)
  {
    resultFields=argValue;
    resultFieldsFlag=true;
  }
  std::vector<DeliveryFields>& getResultFieldsRef()
  {
    resultFieldsFlag=true;
    return resultFields;
  }
  bool hasResultFields()const
  {
    return resultFieldsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!filterFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("filter");
    }
    if(!resultFieldsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resultFields");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(filterTag);
    ds.writeLength(filter.length<DataStream>());
    filter.serialize(ds);
    ds.writeTag(resultFieldsTag);
    ds.writeLength(DataStream::fieldSize(resultFields));
    for(std::vector<DeliveryFields>::const_iterator it=resultFields.begin(),end=resultFields.end();it!=end;++it)
    {
      ds.writeByte(it->getValue());
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
    //  throw protogen::framework::IncompatibleVersionException("GetDeliveriesList");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case filterTag:
        {
          if(filterFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("filter");
          }

          ds.readLength();filter.deserialize(ds);
          filterFlag=true;
        }break;
        case resultFieldsTag:
        {
          if(resultFieldsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("resultFields");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            resultFields.push_back(ds.readByte());
            rd+=DataStream::fieldSize(resultFields.back());
          }
          resultFieldsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetDeliveriesList",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!filterFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("filter");
    }
    if(!resultFieldsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resultFields");
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

  static const int32_t filterTag=1;
  static const int32_t resultFieldsTag=2;

  int32_t seqNum;
  int connId;

  DeliveriesFilter filter;
  std::vector<DeliveryFields> resultFields;

  bool filterFlag;
  bool resultFieldsFlag;
};

}
}
}
}
#endif
