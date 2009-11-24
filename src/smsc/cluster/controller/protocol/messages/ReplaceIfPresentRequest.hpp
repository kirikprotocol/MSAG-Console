#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REPLACEIFPRESENTREQUEST_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REPLACEIFPRESENTREQUEST_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) ReplaceIfPresentRequest version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class ReplaceIfPresentRequest{
public:
  ReplaceIfPresentRequest()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    msgFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(msgFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="msg=";
      rv+="[";
      bool first=true;
      for(std::vector<uint8_t>::const_iterator it=msg.begin(),end=msg.end();it!=end;it++)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        sprintf(buf,"%u",(unsigned int)*it);
        rv+=buf;
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(msgFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(msg);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<uint8_t>& getMsg()const
  {
    if(!msgFlag)
    {
      throw protogen::framework::FieldIsNullException("msg");
    }
    return msg;
  }
  void setMsg(const std::vector<uint8_t>& value)
  {
    msg=value;
    msgFlag=true;
  }
  bool hasMsg()const
  {
    return msgFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!msgFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("msg");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(msgTag);
    ds.writeLength(DataStream::fieldSize(msg));
    for(std::vector<uint8_t>::const_iterator it=msg.begin(),end=msg.end();it!=end;it++)
    {
      ds.writeByte(*it);
    }
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
    //  throw protogen::framework::IncompatibleVersionException("ReplaceIfPresentRequest");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case msgTag:
        {
          if(msgFlag)
          {
            throw protogen::framework::DuplicateFieldException("msg");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            msg.push_back(ds.readByte());
            rd+=DataStream::fieldSize(msg.back());
          }
          msgFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("ReplaceIfPresentRequest",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!msgFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("msg");
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

  static const uint32_t msgTag=1;

  uint32_t seqNum;

  std::vector<uint8_t> msg;

  bool msgFlag;
};

}
}
}
}
}
#endif
