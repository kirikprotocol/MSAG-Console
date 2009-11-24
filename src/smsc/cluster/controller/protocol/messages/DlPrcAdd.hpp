#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLPRCADD_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLPRCADD_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "PrcInfo.hpp"


#ident "@(#) DlPrcAdd version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlPrcAdd{
public:
  DlPrcAdd()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    prcFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(prcFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="prc=";
      rv+='(';
      rv+=prc.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(prcFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=prc.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const PrcInfo& getPrc()const
  {
    if(!prcFlag)
    {
      throw protogen::framework::FieldIsNullException("prc");
    }
    return prc;
  }
  void setPrc(const PrcInfo& value)
  {
    prc=value;
    prcFlag=true;
  }
  bool hasPrc()const
  {
    return prcFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!prcFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("prc");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(prcTag);
    ds.writeLength(prc.length<DataStream>());
    prc.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("DlPrcAdd");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case prcTag:
        {
          if(prcFlag)
          {
            throw protogen::framework::DuplicateFieldException("prc");
          }

          ds.readLength();prc.deserialize(ds);
          prcFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlPrcAdd",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!prcFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("prc");
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

  static const uint32_t prcTag=1;

  uint32_t seqNum;

  PrcInfo prc;

  bool prcFlag;
};

}
}
}
}
}
#endif
