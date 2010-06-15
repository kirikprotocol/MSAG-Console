#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_LOCKCONFIG_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_LOCKCONFIG_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ConfigType.hpp"


#ident "@(#) LockConfig version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class LockConfig{
public:
  LockConfig()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    configTypeFlag=false;
    writeLockFlag=false;
  }
 
  static int32_t getTag()
  {
    return 201;
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
    if(writeLockFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="writeLock=";
      rv+=writeLock?"true":"false";
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
    if(writeLockFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(writeLock);
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
  bool getWriteLock()const
  {
    if(!writeLockFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("writeLock");
    }
    return writeLock;
  }
  void setWriteLock(bool argValue)
  {
    writeLock=argValue;
    writeLockFlag=true;
  }
  bool& getWriteLockRef()
  {
    writeLockFlag=true;
    return writeLock;
  }
  bool hasWriteLock()const
  {
    return writeLockFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!configTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("configType");
    }
    if(!writeLockFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("writeLock");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(configTypeTag);
    ds.writeByteLV(configType);
 
    ds.writeTag(writeLockTag);
    ds.writeBoolLV(writeLock);
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
    //  throw protogen::framework::IncompatibleVersionException("LockConfig");
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
        case writeLockTag:
        {
          if(writeLockFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("writeLock");
          }
          writeLock=ds.readBoolLV();
          writeLockFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("LockConfig",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!configTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("configType");
    }
    if(!writeLockFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("writeLock");
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
  static const int32_t writeLockTag=2;

  int32_t seqNum;

  ConfigType::type configType;
  bool writeLock;

  bool configTypeFlag;
  bool writeLockFlag;
};

}
}
}
}
}
#endif
