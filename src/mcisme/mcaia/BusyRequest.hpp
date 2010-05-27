#ifndef __GENERATED_MESSAGE_SMSC_MCISME_MCAIA_BUSYREQUEST_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_MCAIA_BUSYREQUEST_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) BusyRequest version 1.0"



namespace smsc{
namespace mcisme{
namespace mcaia{


class BusyRequest{
public:
  BusyRequest()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    callerFlag=false;
    calledFlag=false;
    dateFlag=false;
    causeFlag=false;
    flagsFlag=false;
  }
 
  static int32_t getTag()
  {
    return 1;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(callerFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="caller=";
      rv+=caller;
    }
    if(calledFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="called=";
      rv+=called;
    }
    if(dateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="date=";
      sprintf(buf,"%lld",date);
      rv+=buf;
    }
    if(causeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="cause=";
      sprintf(buf,"%d",(int)cause);
      rv+=buf;
    }
    if(flagsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="flags=";
      sprintf(buf,"%d",(int)flags);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(callerFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(caller);
    }
    if(calledFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(called);
    }
    if(dateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(date);
    }
    if(causeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(cause);
    }
    if(flagsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(flags);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getCaller()const
  {
    if(!callerFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("caller");
    }
    return caller;
  }
  void setCaller(const std::string& argValue)
  {
    caller=argValue;
    callerFlag=true;
  }
  std::string& getCallerRef()
  {
    callerFlag=true;
    return caller;
  }
  bool hasCaller()const
  {
    return callerFlag;
  }
  const std::string& getCalled()const
  {
    if(!calledFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("called");
    }
    return called;
  }
  void setCalled(const std::string& argValue)
  {
    called=argValue;
    calledFlag=true;
  }
  std::string& getCalledRef()
  {
    calledFlag=true;
    return called;
  }
  bool hasCalled()const
  {
    return calledFlag;
  }
  int64_t getDate()const
  {
    if(!dateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("date");
    }
    return date;
  }
  void setDate(int64_t argValue)
  {
    date=argValue;
    dateFlag=true;
  }
  int64_t& getDateRef()
  {
    dateFlag=true;
    return date;
  }
  bool hasDate()const
  {
    return dateFlag;
  }
  int8_t getCause()const
  {
    if(!causeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("cause");
    }
    return cause;
  }
  void setCause(int8_t argValue)
  {
    cause=argValue;
    causeFlag=true;
  }
  int8_t& getCauseRef()
  {
    causeFlag=true;
    return cause;
  }
  bool hasCause()const
  {
    return causeFlag;
  }
  int8_t getFlags()const
  {
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("flags");
    }
    return flags;
  }
  void setFlags(int8_t argValue)
  {
    flags=argValue;
    flagsFlag=true;
  }
  int8_t& getFlagsRef()
  {
    flagsFlag=true;
    return flags;
  }
  bool hasFlags()const
  {
    return flagsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!callerFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("caller");
    }
    if(!calledFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("called");
    }
    if(!dateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("date");
    }
    if(!causeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("cause");
    }
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flags");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(callerTag);
    ds.writeStrLV(caller);
    ds.writeTag(calledTag);
    ds.writeStrLV(called);
    ds.writeTag(dateTag);
    ds.writeInt64LV(date);
    ds.writeTag(causeTag);
    ds.writeByteLV(cause);
    ds.writeTag(flagsTag);
    ds.writeByteLV(flags);
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
    //  throw protogen::framework::IncompatibleVersionException("BusyRequest");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case callerTag:
        {
          if(callerFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("caller");
          }
          caller=ds.readStrLV();
          callerFlag=true;
        }break;
        case calledTag:
        {
          if(calledFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("called");
          }
          called=ds.readStrLV();
          calledFlag=true;
        }break;
        case dateTag:
        {
          if(dateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("date");
          }
          date=ds.readInt64LV();
          dateFlag=true;
        }break;
        case causeTag:
        {
          if(causeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("cause");
          }
          cause=ds.readByteLV();
          causeFlag=true;
        }break;
        case flagsTag:
        {
          if(flagsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("flags");
          }
          flags=ds.readByteLV();
          flagsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("BusyRequest",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!callerFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("caller");
    }
    if(!calledFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("called");
    }
    if(!dateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("date");
    }
    if(!causeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("cause");
    }
    if(!flagsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("flags");
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

  static const int32_t callerTag=1;
  static const int32_t calledTag=2;
  static const int32_t dateTag=3;
  static const int32_t causeTag=4;
  static const int32_t flagsTag=5;

  int32_t seqNum;

  std::string caller;
  std::string called;
  int64_t date;
  int8_t cause;
  int8_t flags;

  bool callerFlag;
  bool calledFlag;
  bool dateFlag;
  bool causeFlag;
  bool flagsFlag;
};

}
}
}
#endif
