#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_GETSMSCCONFIGSSTATERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_GETSMSCCONFIGSSTATERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "SmscConfigsState.hpp"


#ident "@(#) GetSmscConfigsStateResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class GetSmscConfigsStateResp{
public:
  GetSmscConfigsStateResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    stateFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 1205;
  }

  static std::string messageGetName()
  {
    return "GetSmscConfigsStateResp";
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
    if(stateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="state=";
      rv+='(';
      rv+=state.toString();
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
    if(stateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=state.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const Response& getResp()const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("resp");
    }
    return resp;
  }
  void setResp(const Response& argValue)
  {
    resp=argValue;
    respFlag=true;
  }
  Response& getRespRef()
  {
    respFlag=true;
    return resp;
  }
  bool hasResp()const
  {
    return respFlag;
  }
  const SmscConfigsState& getState()const
  {
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("state");
    }
    return state;
  }
  void setState(const SmscConfigsState& argValue)
  {
    state=argValue;
    stateFlag=true;
  }
  SmscConfigsState& getStateRef()
  {
    stateFlag=true;
    return state;
  }
  bool hasState()const
  {
    return stateFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("state");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(stateTag);
    ds.writeLength(state.length<DataStream>());
    state.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("GetSmscConfigsStateResp");
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
        case stateTag:
        {
          if(stateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("state");
          }

          ds.readLength();state.deserialize(ds);
          stateFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetSmscConfigsStateResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("state");
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
  static const int32_t stateTag=2;

  int32_t seqNum;

  Response resp;
  SmscConfigsState state;

  bool respFlag;
  bool stateFlag;
};

}
}
}
}
#endif
