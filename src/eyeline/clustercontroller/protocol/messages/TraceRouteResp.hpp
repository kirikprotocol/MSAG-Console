#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_TRACEROUTERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_TRACEROUTERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) TraceRouteResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class TraceRouteResp{
public:
  TraceRouteResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    foundFlag=false;
    routeIdFlag=false;
    traceFlag=false;
    trace.clear();
    aliasInfoFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 1008;
  }

  static std::string messageGetName()
  {
    return "TraceRouteResp";
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
    if(foundFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="found=";
      rv+=found?"true":"false";
    }
    if(routeIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="routeId=";
      rv+=routeId;
    }
    if(traceFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="trace=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=trace.begin(),end=trace.end();it!=end;++it)
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
    if(aliasInfoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="aliasInfo=";
      rv+=aliasInfo;
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
    if(foundFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(found);
    }
    if(routeIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(routeId);
    }
    if(traceFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(trace);
    }
    if(aliasInfoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(aliasInfo);
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
  bool getFound()const
  {
    if(!foundFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("found");
    }
    return found;
  }
  void setFound(bool argValue)
  {
    found=argValue;
    foundFlag=true;
  }
  bool& getFoundRef()
  {
    foundFlag=true;
    return found;
  }
  bool hasFound()const
  {
    return foundFlag;
  }
  const std::string& getRouteId()const
  {
    if(!routeIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("routeId");
    }
    return routeId;
  }
  void setRouteId(const std::string& argValue)
  {
    routeId=argValue;
    routeIdFlag=true;
  }
  std::string& getRouteIdRef()
  {
    routeIdFlag=true;
    return routeId;
  }
  bool hasRouteId()const
  {
    return routeIdFlag;
  }
  const std::vector<std::string>& getTrace()const
  {
    if(!traceFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("trace");
    }
    return trace;
  }
  void setTrace(const std::vector<std::string>& argValue)
  {
    trace=argValue;
    traceFlag=true;
  }
  std::vector<std::string>& getTraceRef()
  {
    traceFlag=true;
    return trace;
  }
  bool hasTrace()const
  {
    return traceFlag;
  }
  const std::string& getAliasInfo()const
  {
    if(!aliasInfoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("aliasInfo");
    }
    return aliasInfo;
  }
  void setAliasInfo(const std::string& argValue)
  {
    aliasInfo=argValue;
    aliasInfoFlag=true;
  }
  std::string& getAliasInfoRef()
  {
    aliasInfoFlag=true;
    return aliasInfo;
  }
  bool hasAliasInfo()const
  {
    return aliasInfoFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!foundFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("found");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(foundTag);
    ds.writeBoolLV(found); 
    if(routeIdFlag)
    {
      ds.writeTag(routeIdTag);
    ds.writeStrLV(routeId); 
    }
    if(traceFlag)
    {
      ds.writeTag(traceTag);
    ds.writeLength(DataStream::fieldSize(trace));
    for(std::vector<std::string>::const_iterator it=trace.begin(),end=trace.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    }
    if(aliasInfoFlag)
    {
      ds.writeTag(aliasInfoTag);
    ds.writeStrLV(aliasInfo); 
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
    //  throw protogen::framework::IncompatibleVersionException("TraceRouteResp");
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
        case foundTag:
        {
          if(foundFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("found");
          }
          found=ds.readBoolLV();
          foundFlag=true;
        }break;
        case routeIdTag:
        {
          if(routeIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("routeId");
          }
          routeId=ds.readStrLV();
          routeIdFlag=true;
        }break;
        case traceTag:
        {
          if(traceFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("trace");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            trace.push_back(ds.readStr());
            rd+=DataStream::fieldSize(trace.back());
            rd+=DataStream::lengthTypeSize;
          }
          traceFlag=true;
        }break;
        case aliasInfoTag:
        {
          if(aliasInfoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("aliasInfo");
          }
          aliasInfo=ds.readStrLV();
          aliasInfoFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("TraceRouteResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!foundFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("found");
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
  static const int32_t foundTag=2;
  static const int32_t routeIdTag=3;
  static const int32_t traceTag=4;
  static const int32_t aliasInfoTag=5;

  int32_t seqNum;

  Response resp;
  bool found;
  std::string routeId;
  std::vector<std::string> trace;
  std::string aliasInfo;

  bool respFlag;
  bool foundFlag;
  bool routeIdFlag;
  bool traceFlag;
  bool aliasInfoFlag;
};

}
}
}
}
#endif
