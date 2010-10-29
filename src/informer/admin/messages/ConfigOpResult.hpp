#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_CONFIGOPRESULT_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_CONFIGOPRESULT_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) ConfigOpResult version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class ConfigOpResult{
public:
  ConfigOpResult()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    statusFlag=false;
    msgFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 2;
  }

  static std::string messageGetName()
  {
    return "ConfigOpResult";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      sprintf(buf,"%d",status);
      rv+=buf;
    }
    if(msgFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="msg=";
      rv+=msg;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status);
    }
    if(msgFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(msg);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(int32_t argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  int32_t& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const std::string& getMsg()const
  {
    if(!msgFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("msg");
    }
    return msg;
  }
  void setMsg(const std::string& argValue)
  {
    msg=argValue;
    msgFlag=true;
  }
  std::string& getMsgRef()
  {
    msgFlag=true;
    return msg;
  }
  bool hasMsg()const
  {
    return msgFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeInt32LV(status); 
    if(msgFlag)
    {
      ds.writeTag(msgTag);
    ds.writeStrLV(msg); 
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
    //  throw protogen::framework::IncompatibleVersionException("ConfigOpResult");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }
          status=ds.readInt32LV();
          statusFlag=true;
        }break;
        case msgTag:
        {
          if(msgFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("msg");
          }
          msg=ds.readStrLV();
          msgFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("ConfigOpResult",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
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

  static const int32_t statusTag=1;
  static const int32_t msgTag=2;

  int32_t seqNum;
  int connId;

  int32_t status;
  std::string msg;

  bool statusFlag;
  bool msgFlag;
};

}
}
}
}
#endif
