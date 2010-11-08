#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETNEXTMESSAGESPACKRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETNEXTMESSAGESPACKRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "MessageInfo.hpp"


#ident "@(#) GetNextMessagesPackResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetNextMessagesPackResp{
public:
  GetNextMessagesPackResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    moreMessagesFlag=false;
    infoFlag=false;
    info.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 46;
  }

  static std::string messageGetName()
  {
    return "GetNextMessagesPackResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(moreMessagesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="moreMessages=";
      rv+=moreMessages?"true":"false";
    }
    if(infoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="info=";
      rv+="[";
      bool first=true;
      for(std::vector<MessageInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+="(";
        rv+=it->toString();
        rv+=")";
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(moreMessagesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(moreMessages);
    }
    if(infoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<MessageInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  bool getMoreMessages()const
  {
    if(!moreMessagesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("moreMessages");
    }
    return moreMessages;
  }
  void setMoreMessages(bool argValue)
  {
    moreMessages=argValue;
    moreMessagesFlag=true;
  }
  bool& getMoreMessagesRef()
  {
    moreMessagesFlag=true;
    return moreMessages;
  }
  bool hasMoreMessages()const
  {
    return moreMessagesFlag;
  }
  const std::vector<MessageInfo>& getInfo()const
  {
    if(!infoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("info");
    }
    return info;
  }
  void setInfo(const std::vector<MessageInfo>& argValue)
  {
    info=argValue;
    infoFlag=true;
  }
  std::vector<MessageInfo>& getInfoRef()
  {
    infoFlag=true;
    return info;
  }
  bool hasInfo()const
  {
    return infoFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!moreMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreMessages");
    }
    if(!infoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("info");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(moreMessagesTag);
    ds.writeBoolLV(moreMessages); 
    ds.writeTag(infoTag);
    typename DataStream::LengthType len=0;
    for(std::vector<MessageInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<MessageInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
    {
      it->serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("GetNextMessagesPackResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case moreMessagesTag:
        {
          if(moreMessagesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("moreMessages");
          }
          moreMessages=ds.readBoolLV();
          moreMessagesFlag=true;
        }break;
        case infoTag:
        {
          if(infoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("info");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            info.push_back(MessageInfo());
            info.back().deserialize(ds);
            rd+=info.back().length<DataStream>();
          }
          infoFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetNextMessagesPackResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!moreMessagesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreMessages");
    }
    if(!infoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("info");
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

  static const int32_t moreMessagesTag=1;
  static const int32_t infoTag=2;

  int32_t seqNum;
  int connId;

  bool moreMessages;
  std::vector<MessageInfo> info;

  bool moreMessagesFlag;
  bool infoFlag;
};

}
}
}
}
#endif
