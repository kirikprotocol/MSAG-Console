#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMSCCONFIGSSTATE_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMSCCONFIGSSTATE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmscConfigsState version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class SmscConfigsState{
public:
  SmscConfigsState()
  {
    Clear();
  }
  void Clear()
  {
    nodeIdexFlag=false;
    updateTimeFlag=false;
    updateTime.clear();
  }
 

  static std::string messageGetName()
  {
    return "SmscConfigsState";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(nodeIdexFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="nodeIdex=";
      sprintf(buf,"%d",(int)nodeIdex);
      rv+=buf;
    }
    if(updateTimeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="updateTime=";
      rv+="[";
      bool first=true;
      for(std::vector<int64_t>::const_iterator it=updateTime.begin(),end=updateTime.end();it!=end;++it)
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
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(nodeIdexFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(nodeIdex);
    }
    if(updateTimeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(updateTime);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int8_t getNodeIdex()const
  {
    if(!nodeIdexFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("nodeIdex");
    }
    return nodeIdex;
  }
  void setNodeIdex(int8_t argValue)
  {
    nodeIdex=argValue;
    nodeIdexFlag=true;
  }
  int8_t& getNodeIdexRef()
  {
    nodeIdexFlag=true;
    return nodeIdex;
  }
  bool hasNodeIdex()const
  {
    return nodeIdexFlag;
  }
  const std::vector<int64_t>& getUpdateTime()const
  {
    if(!updateTimeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("updateTime");
    }
    return updateTime;
  }
  void setUpdateTime(const std::vector<int64_t>& argValue)
  {
    updateTime=argValue;
    updateTimeFlag=true;
  }
  std::vector<int64_t>& getUpdateTimeRef()
  {
    updateTimeFlag=true;
    return updateTime;
  }
  bool hasUpdateTime()const
  {
    return updateTimeFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!nodeIdexFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("nodeIdex");
    }
    if(!updateTimeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("updateTime");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(nodeIdexTag);
    ds.writeByteLV(nodeIdex); 
    ds.writeTag(updateTimeTag);
    ds.writeLength(DataStream::fieldSize(updateTime));
    for(std::vector<int64_t>::const_iterator it=updateTime.begin(),end=updateTime.end();it!=end;++it)
    {
      ds.writeInt64(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("SmscConfigsState");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case nodeIdexTag:
        {
          if(nodeIdexFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("nodeIdex");
          }
          nodeIdex=ds.readByteLV();
          nodeIdexFlag=true;
        }break;
        case updateTimeTag:
        {
          if(updateTimeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("updateTime");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            updateTime.push_back(ds.readInt64());
            rd+=DataStream::fieldSize(updateTime.back());
          }
          updateTimeFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmscConfigsState",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!nodeIdexFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("nodeIdex");
    }
    if(!updateTimeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("updateTime");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t nodeIdexTag=1;
  static const int32_t updateTimeTag=2;


  int8_t nodeIdex;
  std::vector<int64_t> updateTime;

  bool nodeIdexFlag;
  bool updateTimeFlag;
};

}
}
}
}
#endif
