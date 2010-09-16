#ifndef __GENERATED_MESSAGE_EYELINE_LOAD_BALANCER_CLUSTER_MESSAGES_REGISTERASLOADBALANCER_HPP__
#define __GENERATED_MESSAGE_EYELINE_LOAD_BALANCER_CLUSTER_MESSAGES_REGISTERASLOADBALANCER_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) RegisterAsLoadBalancer version 1.0"



namespace eyeline{
namespace load_balancer{
namespace cluster{
namespace messages{


class RegisterAsLoadBalancer{
public:
  RegisterAsLoadBalancer()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    magicFlag=false;
  }
 
  static int32_t getTag()
  {
    return 301;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(magicFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="magic=";
      sprintf(buf,"%d",magic);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(magicFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(magic);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getMagic()const
  {
    if(!magicFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("magic");
    }
    return magic;
  }
  void setMagic(int32_t argValue)
  {
    magic=argValue;
    magicFlag=true;
  }
  int32_t& getMagicRef()
  {
    magicFlag=true;
    return magic;
  }
  bool hasMagic()const
  {
    return magicFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!magicFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("magic");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(magicTag);
    ds.writeInt32LV(magic);
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
    //  throw protogen::framework::IncompatibleVersionException("RegisterAsLoadBalancer");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case magicTag:
        {
          if(magicFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("magic");
          }
          magic=ds.readInt32LV();
          magicFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("RegisterAsLoadBalancer",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!magicFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("magic");
    }

  }

  int32_t getSeqNum()const
  {
    return seqNum;
  }

  void setSeqNum(int32_t argValue)
  {
    seqNum=argValue;
  }

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t magicTag=1;

  int32_t seqNum;

  int32_t magic;

  bool magicFlag;
};

}
}
}
}
#endif
