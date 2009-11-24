#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEDISCONNECT_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEDISCONNECT_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmeDisconnect version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

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
      for(string_list::const_iterator it=sysIds.begin(),end=sysIds.end();it!=end;it++)
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
  uint32_t length()const
  {
    uint32_t rv=0;
    if(sysIdsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(sysIds);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const string_list& getSysIds()const
  {
    if(!sysIdsFlag)
    {
      throw protogen::framework::FieldIsNullException("sysIds");
    }
    return sysIds;
  }
  void setSysIds(const string_list& value)
  {
    sysIds=value;
    sysIdsFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("sysIds");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(sysIdsTag);
    ds.writeStrLstLV(sysIds);
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
    //  throw protogen::framework::IncompatibleVersionException("SmeDisconnect");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case sysIdsTag:
        {
          if(sysIdsFlag)
          {
            throw protogen::framework::DuplicateFieldException("sysIds");
          }
          ds.readStrLstLV(sysIds);
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
      throw protogen::framework::MandatoryFieldMissingException("sysIds");
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

  static const uint32_t sysIdsTag=1;

  uint32_t seqNum;

  string_list sysIds;

  bool sysIdsFlag;
};

}
}
}
}
}
#endif
