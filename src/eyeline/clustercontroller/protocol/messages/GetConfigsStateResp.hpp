#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_GETCONFIGSSTATERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_GETCONFIGSSTATERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "SmscConfigsState.hpp"


#ident "@(#) GetConfigsStateResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class GetConfigsStateResp{
public:
  GetConfigsStateResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    ccConfigUpdateTimeFlag=false;
    ccConfigUpdateTime.clear();
    smscConfigsFlag=false;
    smscConfigs.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 1203;
  }

  static std::string messageGetName()
  {
    return "GetConfigsStateResp";
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
    if(ccConfigUpdateTimeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="ccConfigUpdateTime=";
      rv+="[";
      bool first=true;
      for(std::vector<int64_t>::const_iterator it=ccConfigUpdateTime.begin(),end=ccConfigUpdateTime.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        sprintf(buf,"%lld",*it);
        rv+=buf;
      }
      rv+="]";
    }
    if(smscConfigsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smscConfigs=";
      rv+="[";
      bool first=true;
      for(std::vector<SmscConfigsState>::const_iterator it=smscConfigs.begin(),end=smscConfigs.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+="(";
        rv+=it->toString();
        rv+=")";
      }
      rv+="]";
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
    if(ccConfigUpdateTimeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(ccConfigUpdateTime);
    }
    if(smscConfigsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<SmscConfigsState>::const_iterator it=smscConfigs.begin(),end=smscConfigs.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
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
  const std::vector<int64_t>& getCcConfigUpdateTime()const
  {
    if(!ccConfigUpdateTimeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("ccConfigUpdateTime");
    }
    return ccConfigUpdateTime;
  }
  void setCcConfigUpdateTime(const std::vector<int64_t>& argValue)
  {
    ccConfigUpdateTime=argValue;
    ccConfigUpdateTimeFlag=true;
  }
  std::vector<int64_t>& getCcConfigUpdateTimeRef()
  {
    ccConfigUpdateTimeFlag=true;
    return ccConfigUpdateTime;
  }
  bool hasCcConfigUpdateTime()const
  {
    return ccConfigUpdateTimeFlag;
  }
  const std::vector<SmscConfigsState>& getSmscConfigs()const
  {
    if(!smscConfigsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("smscConfigs");
    }
    return smscConfigs;
  }
  void setSmscConfigs(const std::vector<SmscConfigsState>& argValue)
  {
    smscConfigs=argValue;
    smscConfigsFlag=true;
  }
  std::vector<SmscConfigsState>& getSmscConfigsRef()
  {
    smscConfigsFlag=true;
    return smscConfigs;
  }
  bool hasSmscConfigs()const
  {
    return smscConfigsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!ccConfigUpdateTimeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("ccConfigUpdateTime");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(ccConfigUpdateTimeTag);
    ds.writeLength(DataStream::fieldSize(ccConfigUpdateTime));
    for(std::vector<int64_t>::const_iterator it=ccConfigUpdateTime.begin(),end=ccConfigUpdateTime.end();it!=end;++it)
    {
      ds.writeInt64(*it);
          }
    if(smscConfigsFlag)
    {
      ds.writeTag(smscConfigsTag);
    typename DataStream::LengthType len=0;
    for(std::vector<SmscConfigsState>::const_iterator it=smscConfigs.begin(),end=smscConfigs.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<SmscConfigsState>::const_iterator it=smscConfigs.begin(),end=smscConfigs.end();it!=end;++it)
    {
      it->serialize(ds);
    }
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
    //  throw protogen::framework::IncompatibleVersionException("GetConfigsStateResp");
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
        case ccConfigUpdateTimeTag:
        {
          if(ccConfigUpdateTimeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("ccConfigUpdateTime");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            ccConfigUpdateTime.push_back(ds.readInt64());
            rd+=DataStream::fieldSize(ccConfigUpdateTime.back());
          }
          ccConfigUpdateTimeFlag=true;
        }break;
        case smscConfigsTag:
        {
          if(smscConfigsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("smscConfigs");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            smscConfigs.push_back(SmscConfigsState());
            smscConfigs.back().deserialize(ds);
            rd+=smscConfigs.back().length<DataStream>();
          }
          smscConfigsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetConfigsStateResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!ccConfigUpdateTimeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("ccConfigUpdateTime");
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
  static const int32_t ccConfigUpdateTimeTag=2;
  static const int32_t smscConfigsTag=3;

  int32_t seqNum;

  Response resp;
  std::vector<int64_t> ccConfigUpdateTime;
  std::vector<SmscConfigsState> smscConfigs;

  bool respFlag;
  bool ccConfigUpdateTimeFlag;
  bool smscConfigsFlag;
};

}
}
}
}
#endif
