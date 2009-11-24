#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REGISTERASLOADBALANCER_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REGISTERASLOADBALANCER_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) RegisterAsLoadBalancer version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

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
      sprintf(buf,"%u",(unsigned int)magic);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(magicFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(magic);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  uint32_t getMagic()const
  {
    if(!magicFlag)
    {
      throw protogen::framework::FieldIsNullException("magic");
    }
    return magic;
  }
  void setMagic(uint32_t value)
  {
    magic=value;
    magicFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("magic");
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
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("RegisterAsLoadBalancer");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case magicTag:
        {
          if(magicFlag)
          {
            throw protogen::framework::DuplicateFieldException("magic");
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
      throw protogen::framework::MandatoryFieldMissingException("magic");
    }

  }

  uint32_t getSeqNum()const
  {
    return seqNum;
  }

  void setSeqNum(uint32_t value)
  {
    seqNum=value;
  }

protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t magicTag=1;

  uint32_t seqNum;

  uint32_t magic;

  bool magicFlag;
};

}
}
}
}
}
#endif
