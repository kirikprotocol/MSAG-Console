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
    configUpdateTimesFlag=false;
    configUpdateTimes.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 303;
  }

  static std::string messageGetName()
  {
    return "RegisterAsSmsc";
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
      sprintf(buf,"%d",(int)nodeIndex);
      rv+=buf;
    }
    if(configUpdateTimesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="configUpdateTimes=";
      rv+="[";
      bool first=true;
      for(std::vector<int64_t>::const_iterator it=configUpdateTimes.begin(),end=configUpdateTimes.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        sprintf(buf,"%lld",*it);
        rv+=buf;
      }
      rv+="]";
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
    if(configUpdateTimesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(configUpdateTimes);
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
  int8_t getNodeIndex()const
  {
    if(!nodeIndexFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("nodeIndex");
    }
    return nodeIndex;
  }
  void setNodeIndex(int8_t argValue)
  {
    nodeIndex=argValue;
    nodeIndexFlag=true;
  }
  int8_t& getNodeIndexRef()
  {
    nodeIndexFlag=true;
    return nodeIndex;
  }
  bool hasNodeIndex()const
  {
    return nodeIndexFlag;
  }
  const std::vector<int64_t>& getConfigUpdateTimes()const
  {
    if(!configUpdateTimesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("configUpdateTimes");
    }
    return configUpdateTimes;
  }
  void setConfigUpdateTimes(const std::vector<int64_t>& argValue)
  {
    configUpdateTimes=argValue;
    configUpdateTimesFlag=true;
  }
  std::vector<int64_t>& getConfigUpdateTimesRef()
  {
    configUpdateTimesFlag=true;
    return configUpdateTimes;
  }
  bool hasConfigUpdateTimes()const
  {
    return configUpdateTimesFlag;
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
    if(!configUpdateTimesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("configUpdateTimes");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(magicTag);
    ds.writeInt32LV(magic); 
    ds.writeTag(nodeIndexTag);
    ds.writeByteLV(nodeIndex); 
    ds.writeTag(configUpdateTimesTag);
    ds.writeLength(DataStream::fieldSize(configUpdateTimes));
    for(std::vector<int64_t>::const_iterator it=configUpdateTimes.begin(),end=configUpdateTimes.end();it!=end;++it)
    {
      ds.writeInt64(*it);
          }
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
      typename DataStream::TagType tag=ds.readTag();
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
          nodeIndex=ds.readByteLV();
          nodeIndexFlag=true;
        }break;
        case configUpdateTimesTag:
        {
          if(configUpdateTimesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("configUpdateTimes");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            configUpdateTimes.push_back(ds.readInt64());
            rd+=DataStream::fieldSize(configUpdateTimes.back());
          }
          configUpdateTimesFlag=true;
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
    if(!configUpdateTimesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("configUpdateTimes");
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

  static const int32_t magicTag=1;
  static const int32_t nodeIndexTag=2;
  static const int32_t configUpdateTimesTag=3;

  int32_t seqNum;

  int32_t magic;
  int8_t nodeIndex;
  std::vector<int64_t> configUpdateTimes;

  bool magicFlag;
  bool nodeIndexFlag;
  bool configUpdateTimesFlag;
};

}
}
}
}
}
#endif
