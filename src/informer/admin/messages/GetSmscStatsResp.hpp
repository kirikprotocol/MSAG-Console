#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_GETSMSCSTATSRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_GETSMSCSTATSRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "SmscStats.hpp"


#ident "@(#) GetSmscStatsResp version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class GetSmscStatsResp{
public:
  GetSmscStatsResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    statusFlag=false;
    smscStatsFlag=false;
    smscStats.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 12;
  }

  static std::string messageGetName()
  {
    return "GetSmscStatsResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      sprintf(buf,"%d",status);
      rv+=buf;
    }
    if(smscStatsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smscStats=";
      rv+="[";
      bool first=true;
      for(std::vector<SmscStats>::const_iterator it=smscStats.begin(),end=smscStats.end();it!=end;++it)
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
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status);
    }
    if(smscStatsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<SmscStats>::const_iterator it=smscStats.begin(),end=smscStats.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(int32_t argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  int32_t& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const std::vector<SmscStats>& getSmscStats()const
  {
    if(!smscStatsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("smscStats");
    }
    return smscStats;
  }
  void setSmscStats(const std::vector<SmscStats>& argValue)
  {
    smscStats=argValue;
    smscStatsFlag=true;
  }
  std::vector<SmscStats>& getSmscStatsRef()
  {
    smscStatsFlag=true;
    return smscStats;
  }
  bool hasSmscStats()const
  {
    return smscStatsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!smscStatsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smscStats");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeInt32LV(status); 
    ds.writeTag(smscStatsTag);
    typename DataStream::LengthType len=0;
    for(std::vector<SmscStats>::const_iterator it=smscStats.begin(),end=smscStats.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<SmscStats>::const_iterator it=smscStats.begin(),end=smscStats.end();it!=end;++it)
    {
      it->serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("GetSmscStatsResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }
          status=ds.readInt32LV();
          statusFlag=true;
        }break;
        case smscStatsTag:
        {
          if(smscStatsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("smscStats");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            smscStats.push_back(SmscStats());
            smscStats.back().deserialize(ds);
            rd+=smscStats.back().length<DataStream>();
          }
          smscStatsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetSmscStatsResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!smscStatsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smscStats");
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

  static const int32_t statusTag=1;
  static const int32_t smscStatsTag=2;

  int32_t seqNum;
  int connId;

  int32_t status;
  std::vector<SmscStats> smscStats;

  bool statusFlag;
  bool smscStatsFlag;
};

}
}
}
}
#endif
