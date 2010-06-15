#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_REGISTERASSMSC_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_REGISTERASSMSC_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) RegisterAsSmsc version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


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
 
  static int32_t getTag()
  {
    return 303;
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
    if(nodeIndexFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="nodeIndex=";
      sprintf(buf,"%d",nodeIndex);
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
    if(nodeIndexFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(nodeIndex);
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
  int32_t getNodeIndex()const
  {
    if(!nodeIndexFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("nodeIndex");
    }
    return nodeIndex;
  }
  void setNodeIndex(int32_t argValue)
  {
    nodeIndex=argValue;
    nodeIndexFlag=true;
  }
  int32_t& getNodeIndexRef()
  {
    nodeIndexFlag=true;
    return nodeIndex;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("magic");
    }
    if(!nodeIndexFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("nodeIndex");
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("RegisterAsSmsc");
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
        case nodeIndexTag:
        {
          if(nodeIndexFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("nodeIndex");
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("magic");
    }
    if(!nodeIndexFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("nodeIndex");
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
  static const int32_t nodeIndexTag=2;

  int32_t seqNum;

  int32_t magic;
  int32_t nodeIndex;

  bool magicFlag;
  bool nodeIndexFlag;
};

}
}
}
}
#endif
