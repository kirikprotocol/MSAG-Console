// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_INC_MOD_RESP_HPP__
#define __SCAG_PVSS_PVAP_PC_INC_MOD_RESP_HPP__

#include "util/int.h"
#include <string>
#include "Exceptions.h"


namespace scag{
namespace pvss{
namespace pvap{

class PC_INC_MOD_RESP  {
public:
  PC_INC_MOD_RESP()
  {
    Clear();
  }
  void Clear()
  {
    statusFlag=false;
    intValueFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
		sprintf(buf,"seqNum=%d",seqNum);
		rv+=buf;
    if(statusFlag)
    {
      rv+=";status=";
      sprintf(buf,"%u",(unsigned int)status);
      rv+=buf;
    }
    if(intValueFlag)
    {
      rv+=";intValue=";
      sprintf(buf,"%u",(unsigned int)intValue);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status);
    }
    if(intValueFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(intValue);
    }

    return rv;
  }
  uint8_t getStatus()const
  {
    if(!statusFlag)
    {
      throw FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(uint8_t value)
  {
    status=value;
    statusFlag=true;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  uint32_t getIntValue()const
  {
    if(!intValueFlag)
    {
      throw FieldIsNullException("intValue");
    }
    return intValue;
  }
  void setIntValue(uint32_t value)
  {
    intValue=value;
    intValueFlag=true;
  }
  bool hasIntValue()const
  {
    return intValueFlag;
  }

  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw MandatoryFieldMissingException("status");
    }
    if(!intValueFlag)
    {
      throw MandatoryFieldMissingException("intValue");
    }
    // checking profile type
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeByteLV(status);
    ds.writeTag(intValueTag);
    ds.writeInt32LV(intValue);
    //ds.writeTag(DataStream::endOfMessage_tag);
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
    //  throw IncompatibleVersionException("PC_INC_MOD_RESP");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case statusTag:
        {
          if(statusFlag)
          {
            throw DuplicateFieldException("status");
          }
          status=ds.readByteLV();
          statusFlag=true;
        }break;
        case intValueTag:
        {
          if(intValueFlag)
          {
            throw DuplicateFieldException("intValue");
          }
          intValue=ds.readInt32LV();
          intValueFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw UnexpectedTag("PC_INC_MOD_RESP",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw MandatoryFieldMissingException("status");
    }
    if(!intValueFlag)
    {
      throw MandatoryFieldMissingException("intValue");
    }
    // checking profile type
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
  //static const uint8_t versionMajor=2;
  //static const uint8_t versionMinor=0;

  static const uint32_t statusTag=1;
  static const uint32_t intValueTag=10;

  uint32_t seqNum;

  uint8_t status;
  uint32_t intValue;

  bool statusFlag;
  bool intValueFlag;
};

}
}
}
#endif
