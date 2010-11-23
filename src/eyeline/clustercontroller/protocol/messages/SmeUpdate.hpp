#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEUPDATE_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEUPDATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmeParams.hpp"


#ident "@(#) SmeUpdate version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


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
 
  static int32_t messageGetTag()
  {
    return 19;
  }

  static std::string messageGetName()
  {
    return "SmeUpdate";
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
  int32_t length()const
  {
    int32_t rv=0;
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
      throw eyeline::protogen::framework::FieldIsNullException("params");
    }
    return params;
  }
  void setParams(const SmeParams& argValue)
  {
    params=argValue;
    paramsFlag=true;
  }
  SmeParams& getParamsRef()
  {
    paramsFlag=true;
    return params;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("params");
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("SmeUpdate");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case paramsTag:
        {
          if(paramsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("params");
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("params");
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

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t paramsTag=1;

  int32_t seqNum;

  SmeParams params;

  bool paramsFlag;
};

}
}
}
}
#endif
