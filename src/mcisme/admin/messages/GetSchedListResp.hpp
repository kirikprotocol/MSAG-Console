#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSCHEDLISTRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETSCHEDLISTRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "TimeLineInfo.hpp"


#ident "@(#) GetSchedListResp version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class GetSchedListResp:public Response{
public:
  GetSchedListResp()
  {
    Clear();
  }
  void Clear()
  {
    Response::Clear();
    timeLinesFlag=false;
    timeLines.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 13;
  }

  static std::string messageGetName()
  {
    return "GetSchedListResp";
  }

  std::string toString()const
  {
    std::string rv;
    rv+=Response::toString();
    if(timeLinesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="timeLines=";
      rv+="[";
      bool first=true;
      for(std::vector<TimeLineInfo>::const_iterator it=timeLines.begin(),end=timeLines.end();it!=end;++it)
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
    rv+=Response::length<DataStream>();
    if(timeLinesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<TimeLineInfo>::const_iterator it=timeLines.begin(),end=timeLines.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<TimeLineInfo>& getTimeLines()const
  {
    if(!timeLinesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("timeLines");
    }
    return timeLines;
  }
  void setTimeLines(const std::vector<TimeLineInfo>& argValue)
  {
    timeLines=argValue;
    timeLinesFlag=true;
  }
  std::vector<TimeLineInfo>& getTimeLinesRef()
  {
    timeLinesFlag=true;
    return timeLines;
  }
  bool hasTimeLines()const
  {
    return timeLinesFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    Response::serialize(ds);
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(timeLinesFlag)
    {
      ds.writeTag(timeLinesTag);
    typename DataStream::LengthType len=0;
    for(std::vector<TimeLineInfo>::const_iterator it=timeLines.begin(),end=timeLines.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<TimeLineInfo>::const_iterator it=timeLines.begin(),end=timeLines.end();it!=end;++it)
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
    Response::deserialize(ds);
    bool endOfMessage=false;
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("GetSchedListResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case timeLinesTag:
        {
          if(timeLinesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("timeLines");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            timeLines.push_back(TimeLineInfo());
            timeLines.back().deserialize(ds);
            rd+=timeLines.back().length<DataStream>();
          }
          timeLinesFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetSchedListResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }

  }


  void messageSetConnId(int argConnId)
  {
    connId=argConnId;
  }

  int messageGetConnId()const
  {
    return connId;
  }
 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t timeLinesTag=1;

  int connId;

  std::vector<TimeLineInfo> timeLines;

  bool timeLinesFlag;
};

}
}
}
}
#endif
