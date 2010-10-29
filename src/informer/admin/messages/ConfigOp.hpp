#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_CONFIGOP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_CONFIGOP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ConfigId.hpp"
#include "ConfigOpId.hpp"


#ident "@(#) ConfigOp version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class ConfigOp{
public:
  ConfigOp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    objNameFlag=false;
    objIdFlag=false;
    cfgIdFlag=false;
    opFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 1;
  }

  static std::string messageGetName()
  {
    return "ConfigOp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(objNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="objName=";
      rv+=objName;
    }
    if(objIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="objId=";
      sprintf(buf,"%d",objId);
      rv+=buf;
    }
    if(cfgIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="cfgId=";
      rv+=ConfigId::getNameByValue(cfgId);
    }
    if(opFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="op=";
      rv+=ConfigOpId::getNameByValue(op);
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(objNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(objName);
    }
    if(objIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(objId);
    }
    if(cfgIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(cfgId.getValue());
 
    }
    if(opFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(op.getValue());
 
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getObjName()const
  {
    if(!objNameFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("objName");
    }
    return objName;
  }
  void setObjName(const std::string& argValue)
  {
    objName=argValue;
    objNameFlag=true;
  }
  std::string& getObjNameRef()
  {
    objNameFlag=true;
    return objName;
  }
  bool hasObjName()const
  {
    return objNameFlag;
  }
  int32_t getObjId()const
  {
    if(!objIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("objId");
    }
    return objId;
  }
  void setObjId(int32_t argValue)
  {
    objId=argValue;
    objIdFlag=true;
  }
  int32_t& getObjIdRef()
  {
    objIdFlag=true;
    return objId;
  }
  bool hasObjId()const
  {
    return objIdFlag;
  }
  const ConfigId& getCfgId()const
  {
    if(!cfgIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("cfgId");
    }
    return cfgId;
  }
  void setCfgId(const ConfigId& argValue)
  {
    cfgId=argValue;
    cfgIdFlag=true;
  }
  ConfigId& getCfgIdRef()
  {
    cfgIdFlag=true;
    return cfgId;
  }
  bool hasCfgId()const
  {
    return cfgIdFlag;
  }
  const ConfigOpId& getOp()const
  {
    if(!opFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("op");
    }
    return op;
  }
  void setOp(const ConfigOpId& argValue)
  {
    op=argValue;
    opFlag=true;
  }
  ConfigOpId& getOpRef()
  {
    opFlag=true;
    return op;
  }
  bool hasOp()const
  {
    return opFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!cfgIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("cfgId");
    }
    if(!opFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("op");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(cfgIdTag);
    ds.writeByteLV(cfgId.getValue());
 
    ds.writeTag(opTag);
    ds.writeByteLV(op.getValue());
 
    if(objNameFlag)
    {
      ds.writeTag(objNameTag);
    ds.writeStrLV(objName); 
    }
    if(objIdFlag)
    {
      ds.writeTag(objIdTag);
    ds.writeInt32LV(objId); 
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
    //  throw protogen::framework::IncompatibleVersionException("ConfigOp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case objNameTag:
        {
          if(objNameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("objName");
          }
          objName=ds.readStrLV();
          objNameFlag=true;
        }break;
        case objIdTag:
        {
          if(objIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("objId");
          }
          objId=ds.readInt32LV();
          objIdFlag=true;
        }break;
        case cfgIdTag:
        {
          if(cfgIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("cfgId");
          }
          cfgId=ds.readByteLV();
          cfgIdFlag=true;
        }break;
        case opTag:
        {
          if(opFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("op");
          }
          op=ds.readByteLV();
          opFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("ConfigOp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!cfgIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("cfgId");
    }
    if(!opFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("op");
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

  void messageSetConnId(int argConnId)
  {
    connId=argConnId;
  }

  int messageGetConnId()const
  {
    return connId;
  }
 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t objNameTag=1;
  static const int32_t objIdTag=2;
  static const int32_t cfgIdTag=3;
  static const int32_t opTag=4;

  int32_t seqNum;
  int connId;

  std::string objName;
  int32_t objId;
  ConfigId cfgId;
  ConfigOpId op;

  bool objNameFlag;
  bool objIdFlag;
  bool cfgIdFlag;
  bool opFlag;
};

}
}
}
}
#endif
