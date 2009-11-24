#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REGISTERASSMSC_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REGISTERASSMSC_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) RegisterAsSmsc version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class RegisterAsSmsc{
public:
  RegisterAsSmsc()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    magicFlag=false;
    nodeIndexFlag=false;
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
    if(nodeIndexFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="nodeIndex=";
      sprintf(buf,"%u",(unsigned int)nodeIndex);
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
    if(nodeIndexFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(nodeIndex);
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
  uint32_t getNodeIndex()const
  {
    if(!nodeIndexFlag)
    {
      throw protogen::framework::FieldIsNullException("nodeIndex");
    }
    return nodeIndex;
  }
  void setNodeIndex(uint32_t value)
  {
    nodeIndex=value;
    nodeIndexFlag=true;
  }
  bool hasNodeIndex()const
  {
    return nodeIndexFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!magicFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("magic");
    }
    if(!nodeIndexFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("nodeIndex");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(magicTag);
    ds.writeInt32LV(magic);
    ds.writeTag(nodeIndexTag);
    ds.writeInt32LV(nodeIndex);
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
    //  throw protogen::framework::IncompatibleVersionException("RegisterAsSmsc");
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
        case nodeIndexTag:
        {
          if(nodeIndexFlag)
          {
            throw protogen::framework::DuplicateFieldException("nodeIndex");
          }
          nodeIndex=ds.readInt32LV();
          nodeIndexFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("RegisterAsSmsc",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!magicFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("magic");
    }
    if(!nodeIndexFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("nodeIndex");
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
  static const uint32_t nodeIndexTag=2;

  uint32_t seqNum;

  uint32_t magic;
  uint32_t nodeIndex;

  bool magicFlag;
  bool nodeIndexFlag;
};

}
}
}
}
}
#endif
