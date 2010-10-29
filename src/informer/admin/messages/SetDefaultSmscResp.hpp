#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SETDEFAULTSMSCRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SETDEFAULTSMSCRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SetDefaultSmscResp version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class SetDefaultSmscResp{
public:
  SetDefaultSmscResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    statusFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 8;
  }

  static std::string messageGetName()
  {
    return "SetDefaultSmscResp";
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
    //  throw protogen::framework::IncompatibleVersionException("SetDefaultSmscResp");
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
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SetDefaultSmscResp",tag);
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

  int32_t seqNum;
  int connId;

  int32_t status;

  bool statusFlag;
};

}
}
}
}
#endif
