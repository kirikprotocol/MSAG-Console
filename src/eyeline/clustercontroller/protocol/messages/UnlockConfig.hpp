#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_UNLOCKCONFIG_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_UNLOCKCONFIG_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ConfigType.hpp"


#ident "@(#) UnlockConfig version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class UnlockConfig{
public:
  UnlockConfig()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    configTypeFlag=false;
  }
 
  static int32_t getTag()
  {
    return 202;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(configTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="configType=";
      rv+=ConfigType::getNameByValue(configType);
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(configTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(configType);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const ConfigType::type& getConfigType()const
  {
    if(!configTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("configType");
    }
    return configType;
  }
  void setConfigType(const ConfigType::type& argValue)
  {
    if(!ConfigType::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ConfigType",argValue);
    }
    configType=argValue;
    configTypeFlag=true;
  }
  ConfigType::type& getConfigTypeRef()
  {
    configTypeFlag=true;
    return configType;
  }
  bool hasConfigType()const
  {
    return configTypeFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!configTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("configType");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(configTypeTag);
    ds.writeByteLV(configType);
 
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
    //  throw protogen::framework::IncompatibleVersionException("UnlockConfig");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case configTypeTag:
        {
          if(configTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("configType");
          }
          configType=ds.readByteLV();
          configTypeFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("UnlockConfig",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!configTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("configType");
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

  static const int32_t configTypeTag=1;

  int32_t seqNum;

  ConfigType::type configType;

  bool configTypeFlag;
};

}
}
}
}
#endif
