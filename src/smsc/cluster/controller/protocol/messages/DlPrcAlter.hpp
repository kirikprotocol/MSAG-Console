#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLPRCALTER_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLPRCALTER_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlPrcAlter version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlPrcAlter{
public:
  DlPrcAlter()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    addressFlag=false;
    maxListsFlag=false;
    maxElementsFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(addressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="address=";
      rv+=address;
    }
    if(maxListsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxLists=";
      sprintf(buf,"%u",(unsigned int)maxLists);
      rv+=buf;
    }
    if(maxElementsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxElements=";
      sprintf(buf,"%u",(unsigned int)maxElements);
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
    if(maxListsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxLists);
    }
    if(maxElementsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxElements);
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
  uint32_t getMaxLists()const
  {
    if(!maxListsFlag)
    {
      throw protogen::framework::FieldIsNullException("maxLists");
    }
    return maxLists;
  }
  void setMaxLists(uint32_t value)
  {
    maxLists=value;
    maxListsFlag=true;
  }
  bool hasMaxLists()const
  {
    return maxListsFlag;
  }
  uint32_t getMaxElements()const
  {
    if(!maxElementsFlag)
    {
      throw protogen::framework::FieldIsNullException("maxElements");
    }
    return maxElements;
  }
  void setMaxElements(uint32_t value)
  {
    maxElements=value;
    maxElementsFlag=true;
  }
  bool hasMaxElements()const
  {
    return maxElementsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!addressFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("address");
    }
    if(!maxListsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxLists");
    }
    if(!maxElementsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxElements");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(addressTag);
    ds.writeStrLV(address);
    ds.writeTag(maxListsTag);
    ds.writeInt32LV(maxLists);
    ds.writeTag(maxElementsTag);
    ds.writeInt32LV(maxElements);
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
    //  throw protogen::framework::IncompatibleVersionException("DlPrcAlter");
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
        case maxListsTag:
        {
          if(maxListsFlag)
          {
            throw protogen::framework::DuplicateFieldException("maxLists");
          }
          maxLists=ds.readInt32LV();
          maxListsFlag=true;
        }break;
        case maxElementsTag:
        {
          if(maxElementsFlag)
          {
            throw protogen::framework::DuplicateFieldException("maxElements");
          }
          maxElements=ds.readInt32LV();
          maxElementsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlPrcAlter",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!addressFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("address");
    }
    if(!maxListsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxLists");
    }
    if(!maxElementsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("maxElements");
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

  static const uint32_t addressTag=1;
  static const uint32_t maxListsTag=2;
  static const uint32_t maxElementsTag=3;

  uint32_t seqNum;

  std::string address;
  uint32_t maxLists;
  uint32_t maxElements;

  bool addressFlag;
  bool maxListsFlag;
  bool maxElementsFlag;
};

}
}
}
}
}
#endif
