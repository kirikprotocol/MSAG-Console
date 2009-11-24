#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ALIASADD_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_ALIASADD_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AliasAdd version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class AliasAdd{
public:
  AliasAdd()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    addrFlag=false;
    aliasFlag=false;
    hideFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(addrFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="addr=";
      rv+=addr;
    }
    if(aliasFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="alias=";
      rv+=alias;
    }
    if(hideFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="hide=";
      rv+=hide?"true":"false";
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(addrFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(addr);
    }
    if(aliasFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(alias);
    }
    if(hideFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(hide);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
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
  const std::string& getAlias()const
  {
    if(!aliasFlag)
    {
      throw protogen::framework::FieldIsNullException("alias");
    }
    return alias;
  }
  void setAlias(const std::string& value)
  {
    alias=value;
    aliasFlag=true;
  }
  bool hasAlias()const
  {
    return aliasFlag;
  }
  bool getHide()const
  {
    if(!hideFlag)
    {
      throw protogen::framework::FieldIsNullException("hide");
    }
    return hide;
  }
  void setHide(bool value)
  {
    hide=value;
    hideFlag=true;
  }
  bool hasHide()const
  {
    return hideFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!addrFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addr");
    }
    if(!aliasFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("alias");
    }
    if(!hideFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("hide");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(addrTag);
    ds.writeStrLV(addr);
    ds.writeTag(aliasTag);
    ds.writeStrLV(alias);
    ds.writeTag(hideTag);
    ds.writeBoolLV(hide);
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
    //  throw protogen::framework::IncompatibleVersionException("AliasAdd");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case addrTag:
        {
          if(addrFlag)
          {
            throw protogen::framework::DuplicateFieldException("addr");
          }
          addr=ds.readStrLV();
          addrFlag=true;
        }break;
        case aliasTag:
        {
          if(aliasFlag)
          {
            throw protogen::framework::DuplicateFieldException("alias");
          }
          alias=ds.readStrLV();
          aliasFlag=true;
        }break;
        case hideTag:
        {
          if(hideFlag)
          {
            throw protogen::framework::DuplicateFieldException("hide");
          }
          hide=ds.readBoolLV();
          hideFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AliasAdd",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!addrFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("addr");
    }
    if(!aliasFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("alias");
    }
    if(!hideFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("hide");
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

  static const uint32_t addrTag=1;
  static const uint32_t aliasTag=2;
  static const uint32_t hideTag=3;

  uint32_t seqNum;

  std::string addr;
  std::string alias;
  bool hide;

  bool addrFlag;
  bool aliasFlag;
  bool hideFlag;
};

}
}
}
}
}
#endif
