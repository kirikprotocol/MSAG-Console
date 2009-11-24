#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_PRCINFO_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_PRCINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) PrcInfo version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class PrcInfo{
public:
  PrcInfo()
  {
    Clear();
  }
  void Clear()
  {
    addressFlag=false;
    maxLstFlag=false;
    maxElFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(addressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="address=";
      rv+=address;
    }
    if(maxLstFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxLst=";
      sprintf(buf,"%u",(unsigned int)maxLst);
      rv+=buf;
    }
    if(maxElFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxEl=";
      sprintf(buf,"%u",(unsigned int)maxEl);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(addressFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(address);
    }
    if(maxLstFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxLst);
    }
    if(maxElFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxEl);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
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
  uint32_t getMaxLst()const
  {
    if(!maxLstFlag)
    {
      throw protogen::framework::FieldIsNullException("maxLst");
    }
    return maxLst;
  }
  void setMaxLst(uint32_t value)
  {
    maxLst=value;
    maxLstFlag=true;
  }
  bool hasMaxLst()const
  {
    return maxLstFlag;
  }
  uint32_t getMaxEl()const
  {
    if(!maxElFlag)
    {
      throw protogen::framework::FieldIsNullException("maxEl");
    }
    return maxEl;
  }
  void setMaxEl(uint32_t value)
  {
    maxEl=value;
    maxElFlag=true;
  }
  bool hasMaxEl()const
  {
    return maxElFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!addressFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("address");
    }
    if(!maxLstFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxLst");
    }
    if(!maxElFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxEl");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(addressTag);
    ds.writeStrLV(address);
    ds.writeTag(maxLstTag);
    ds.writeInt32LV(maxLst);
    ds.writeTag(maxElTag);
    ds.writeInt32LV(maxEl);
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
    //  throw protogen::framework::IncompatibleVersionException("PrcInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case addressTag:
        {
          if(addressFlag)
          {
            throw protogen::framework::DuplicateFieldException("address");
          }
          address=ds.readStrLV();
          addressFlag=true;
        }break;
        case maxLstTag:
        {
          if(maxLstFlag)
          {
            throw protogen::framework::DuplicateFieldException("maxLst");
          }
          maxLst=ds.readInt32LV();
          maxLstFlag=true;
        }break;
        case maxElTag:
        {
          if(maxElFlag)
          {
            throw protogen::framework::DuplicateFieldException("maxEl");
          }
          maxEl=ds.readInt32LV();
          maxElFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("PrcInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!addressFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("address");
    }
    if(!maxLstFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxLst");
    }
    if(!maxElFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxEl");
    }

  }


protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t addressTag=1;
  static const uint32_t maxLstTag=2;
  static const uint32_t maxElTag=3;


  std::string address;
  uint32_t maxLst;
  uint32_t maxEl;

  bool addressFlag;
  bool maxLstFlag;
  bool maxElFlag;
};

}
}
}
}
}
#endif
