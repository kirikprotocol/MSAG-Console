#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMADDADDR_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMADDADDR_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CgmAddAddr version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class CgmAddAddr{
public:
  CgmAddAddr()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    idFlag=false;
    addrFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 35;
  }

  static std::string messageGetName()
  {
    return "CgmAddAddr";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(idFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="id=";
      sprintf(buf,"%d",id);
      rv+=buf;
    }
    if(addrFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addr=";
      rv+=addr;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(idFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(id);
    }
    if(addrFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addr);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getId()const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(int32_t argValue)
  {
    id=argValue;
    idFlag=true;
  }
  int32_t& getIdRef()
  {
    idFlag=true;
    return id;
  }
  bool hasId()const
  {
    return idFlag;
  }
  const std::string& getAddr()const
  {
    if(!addrFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("addr");
    }
    return addr;
  }
  void setAddr(const std::string& argValue)
  {
    addr=argValue;
    addrFlag=true;
  }
  std::string& getAddrRef()
  {
    addrFlag=true;
    return addr;
  }
  bool hasAddr()const
  {
    return addrFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!addrFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addr");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeInt32LV(id); 
    ds.writeTag(addrTag);
    ds.writeStrLV(addr); 
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
    //  throw protogen::framework::IncompatibleVersionException("CgmAddAddr");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readInt32LV();
          idFlag=true;
        }break;
        case addrTag:
        {
          if(addrFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("addr");
          }
          addr=ds.readStrLV();
          addrFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CgmAddAddr",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!addrFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("addr");
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

  static const int32_t idTag=1;
  static const int32_t addrTag=2;

  int32_t seqNum;

  int32_t id;
  std::string addr;

  bool idFlag;
  bool addrFlag;
};

}
}
}
}
}
#endif
