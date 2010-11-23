#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_APPLYLOCALERESOURCERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_APPLYLOCALERESOURCERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "MultiResponse.hpp"


#ident "@(#) ApplyLocaleResourceResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class ApplyLocaleResourceResp{
public:
  ApplyLocaleResourceResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 1003;
  }

  static std::string messageGetName()
  {
    return "ApplyLocaleResourceResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(respFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="resp=";
      rv+='(';
      rv+=resp.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(respFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=resp.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const MultiResponse& getResp()const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("resp");
    }
    return resp;
  }
  void setResp(const MultiResponse& argValue)
  {
    resp=argValue;
    respFlag=true;
  }
  MultiResponse& getRespRef()
  {
    respFlag=true;
    return resp;
  }
  bool hasResp()const
  {
    return respFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("ApplyLocaleResourceResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case respTag:
        {
          if(respFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("resp");
          }

          ds.readLength();resp.deserialize(ds);
          respFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("ApplyLocaleResourceResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
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

  static const int32_t respTag=1;

  int32_t seqNum;

  MultiResponse resp;

  bool respFlag;
};

}
}
}
}
#endif
