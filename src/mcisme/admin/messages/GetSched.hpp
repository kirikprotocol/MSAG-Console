#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSCHED_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSCHED_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) GetSched version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class GetSched{
public:
  GetSched()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    subscriberFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 8;
  }

  static std::string messageGetName()
  {
    return "GetSched";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(subscriberFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="subscriber=";
      rv+=subscriber;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(subscriberFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(subscriber);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSubscriber()const
  {
    if(!subscriberFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("subscriber");
    }
    return subscriber;
  }
  void setSubscriber(const std::string& argValue)
  {
    subscriber=argValue;
    subscriberFlag=true;
  }
  std::string& getSubscriberRef()
  {
    subscriberFlag=true;
    return subscriber;
  }
  bool hasSubscriber()const
  {
    return subscriberFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!subscriberFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("subscriber");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(subscriberTag);
    ds.writeStrLV(subscriber); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetSched");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case subscriberTag:
        {
          if(subscriberFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("subscriber");
          }
          subscriber=ds.readStrLV();
          subscriberFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetSched",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!subscriberFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("subscriber");
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

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t subscriberTag=1;

  int32_t seqNum;

  std::string subscriber;

  bool subscriberFlag;
};

}
}
}
}
#endif
