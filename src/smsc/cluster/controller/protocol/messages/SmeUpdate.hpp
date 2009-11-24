#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEUPDATE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMEUPDATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmeParams.hpp"


#ident "@(#) SmeUpdate version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class SmeUpdate{
public:
  SmeUpdate()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    paramsFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(paramsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="params=";
      rv+='(';
      rv+=params.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(paramsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=params.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const SmeParams& getParams()const
  {
    if(!paramsFlag)
    {
      throw protogen::framework::FieldIsNullException("params");
    }
    return params;
  }
  void setParams(const SmeParams& value)
  {
    params=value;
    paramsFlag=true;
  }
  bool hasParams()const
  {
    return paramsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!paramsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("params");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(paramsTag);
    ds.writeLength(params.length<DataStream>());
    params.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("SmeUpdate");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case paramsTag:
        {
          if(paramsFlag)
          {
            throw protogen::framework::DuplicateFieldException("params");
          }

          ds.readLength();params.deserialize(ds);
          paramsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmeUpdate",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!paramsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("params");
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

  static const uint32_t paramsTag=1;

  uint32_t seqNum;

  SmeParams params;

  bool paramsFlag;
};

}
}
}
}
}
#endif
