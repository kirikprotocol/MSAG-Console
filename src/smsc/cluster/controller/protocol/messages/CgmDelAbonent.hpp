#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMDELABONENT_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMDELABONENT_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CgmDelAbonent version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class CgmDelAbonent{
public:
  CgmDelAbonent()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    idFlag=false;
    addrFlag=false;
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
      sprintf(buf,"%u",(unsigned int)id);
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
  uint32_t length()const
  {
    uint32_t rv=0;
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
  uint32_t getId()const
  {
    if(!idFlag)
    {
      throw protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(uint32_t value)
  {
    id=value;
    idFlag=true;
  }
  bool hasId()const
  {
    return idFlag;
  }
  const std::string& getAddr()const
  {
    if(!addrFlag)
    {
      throw protogen::framework::FieldIsNullException("addr");
    }
    return addr;
  }
  void setAddr(const std::string& value)
  {
    addr=value;
    addrFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!addrFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addr");
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
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("CgmDelAbonent");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readInt32LV();
          idFlag=true;
        }break;
        case addrTag:
        {
          if(addrFlag)
          {
            throw protogen::framework::DuplicateFieldException("addr");
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
          //  throw protogen::framework::UnexpectedTag("CgmDelAbonent",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("id");
    }
    if(!addrFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addr");
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

  static const uint32_t idTag=1;
  static const uint32_t addrTag=2;

  uint32_t seqNum;

  uint32_t id;
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
