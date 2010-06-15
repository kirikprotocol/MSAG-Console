#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEDISCONNECT_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEDISCONNECT_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmeDisconnect version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class SmeDisconnect{
public:
  SmeDisconnect()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    sysIdsFlag=false;
    sysIds.clear();
  }
 
  static int32_t getTag()
  {
    return 22;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(sysIdsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="sysIds=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=sysIds.begin(),end=sysIds.end();it!=end;it++)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(sysIdsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(sysIds);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<std::string>& getSysIds()const
  {
    if(!sysIdsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("sysIds");
    }
    return sysIds;
  }
  void setSysIds(const std::vector<std::string>& argValue)
  {
    sysIds=argValue;
    sysIdsFlag=true;
  }
  std::vector<std::string>& getSysIdsRef()
  {
    sysIdsFlag=true;
    return sysIds;
  }
  bool hasSysIds()const
  {
    return sysIdsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!sysIdsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("sysIds");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(sysIdsTag);
    ds.writeLength(DataStream::fieldSize(sysIds));
    for(std::vector<std::string>::const_iterator it=sysIds.begin(),end=sysIds.end();it!=end;it++)
    {
      ds.writeStr(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("SmeDisconnect");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case sysIdsTag:
        {
          if(sysIdsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("sysIds");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            sysIds.push_back(ds.readStr());
            rd+=DataStream::fieldSize(sysIds.back());
          }
          sysIdsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeDisconnect",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!sysIdsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("sysIds");
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

  static const int32_t sysIdsTag=1;

  int32_t seqNum;

  std::vector<std::string> sysIds;

  bool sysIdsFlag;
};

}
}
}
}
#endif
