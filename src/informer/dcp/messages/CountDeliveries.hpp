#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIES_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIES_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveriesFilter.hpp"


#ident "@(#) CountDeliveries version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class CountDeliveries{
public:
  CountDeliveries()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    filterFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 54;
  }

  static std::string messageGetName()
  {
    return "CountDeliveries";
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
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!filterFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("filter");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(filterTag);
    ds.writeLength(filter.length<DataStream>());
    filter.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("CountDeliveries");
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
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CountDeliveries",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!filterFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("filter");
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

  int32_t seqNum;
  int connId;

  DeliveriesFilter filter;

  bool filterFlag;
};

}
}
}
}
#endif
