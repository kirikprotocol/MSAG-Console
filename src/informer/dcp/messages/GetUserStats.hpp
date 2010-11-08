#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETUSERSTATS_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETUSERSTATS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) GetUserStats version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetUserStats{
public:
  GetUserStats()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    userIdFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 12;
  }

  static std::string messageGetName()
  {
    return "GetUserStats";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(userIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="userId=";
      rv+=userId;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(userIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userId);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getUserId()const
  {
    if(!userIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("userId");
    }
    return userId;
  }
  void setUserId(const std::string& argValue)
  {
    userId=argValue;
    userIdFlag=true;
  }
  std::string& getUserIdRef()
  {
    userIdFlag=true;
    return userId;
  }
  bool hasUserId()const
  {
    return userIdFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(userIdFlag)
    {
      ds.writeTag(userIdTag);
    ds.writeStrLV(userId); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetUserStats");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case userIdTag:
        {
          if(userIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("userId");
          }
          userId=ds.readStrLV();
          userIdFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetUserStats",tag);
          //}
          ds.skip(ds.readLength());
      }
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

  static const int32_t userIdTag=1;

  int32_t seqNum;
  int connId;

  std::string userId;

  bool userIdFlag;
};

}
}
}
}
#endif
