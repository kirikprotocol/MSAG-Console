#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLSBMDEL_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLSBMDEL_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlSbmDel version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlSbmDel{
public:
  DlSbmDel()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    dlNameFlag=false;
    addressFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(dlNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="dlName=";
      rv+=dlName;
    }
    if(addressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="address=";
      rv+=address;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(dlNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dlName);
    }
    if(addressFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(address);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getDlName()const
  {
    if(!dlNameFlag)
    {
      throw protogen::framework::FieldIsNullException("dlName");
    }
    return dlName;
  }
  void setDlName(const std::string& value)
  {
    dlName=value;
    dlNameFlag=true;
  }
  bool hasDlName()const
  {
    return dlNameFlag;
  }
  const std::string& getAddress()const
  {
    if(!addressFlag)
    {
      throw protogen::framework::FieldIsNullException("address");
    }
    return address;
  }
  void setAddress(const std::string& value)
  {
    address=value;
    addressFlag=true;
  }
  bool hasAddress()const
  {
    return addressFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!dlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dlName");
    }
    if(!addressFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("address");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(dlNameTag);
    ds.writeStrLV(dlName);
    ds.writeTag(addressTag);
    ds.writeStrLV(address);
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
    //  throw protogen::framework::IncompatibleVersionException("DlSbmDel");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case dlNameTag:
        {
          if(dlNameFlag)
          {
            throw protogen::framework::DuplicateFieldException("dlName");
          }
          dlName=ds.readStrLV();
          dlNameFlag=true;
        }break;
        case addressTag:
        {
          if(addressFlag)
          {
            throw protogen::framework::DuplicateFieldException("address");
          }
          address=ds.readStrLV();
          addressFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlSbmDel",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!dlNameFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dlName");
    }
    if(!addressFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("address");
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

  static const uint32_t dlNameTag=1;
  static const uint32_t addressTag=2;

  uint32_t seqNum;

  std::string dlName;
  std::string address;

  bool dlNameFlag;
  bool addressFlag;
};

}
}
}
}
}
#endif
