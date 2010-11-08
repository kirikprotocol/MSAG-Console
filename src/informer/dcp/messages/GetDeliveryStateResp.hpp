#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERYSTATERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERYSTATERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryState.hpp"
#include "DeliveryStatistics.hpp"


#ident "@(#) GetDeliveryStateResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetDeliveryStateResp{
public:
  GetDeliveryStateResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    stateFlag=false;
    statsFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 42;
  }

  static std::string messageGetName()
  {
    return "GetDeliveryStateResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
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
    if(statsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="stats=";
      rv+='(';
      rv+=stats.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(stateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=state.length<DataStream>();
    }
    if(statsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=stats.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const DeliveryState& getState()const
  {
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("state");
    }
    return state;
  }
  void setState(const DeliveryState& argValue)
  {
    state=argValue;
    stateFlag=true;
  }
  DeliveryState& getStateRef()
  {
    stateFlag=true;
    return state;
  }
  bool hasState()const
  {
    return stateFlag;
  }
  const DeliveryStatistics& getStats()const
  {
    if(!statsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("stats");
    }
    return stats;
  }
  void setStats(const DeliveryStatistics& argValue)
  {
    stats=argValue;
    statsFlag=true;
  }
  DeliveryStatistics& getStatsRef()
  {
    statsFlag=true;
    return stats;
  }
  bool hasStats()const
  {
    return statsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("state");
    }
    if(!statsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("stats");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(stateTag);
    ds.writeLength(state.length<DataStream>());
    state.serialize(ds);
    ds.writeTag(statsTag);
    ds.writeLength(stats.length<DataStream>());
    stats.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("GetDeliveryStateResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case stateTag:
        {
          if(stateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("state");
          }

          ds.readLength();state.deserialize(ds);
          stateFlag=true;
        }break;
        case statsTag:
        {
          if(statsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("stats");
          }

          ds.readLength();stats.deserialize(ds);
          statsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetDeliveryStateResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("state");
    }
    if(!statsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("stats");
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

  static const int32_t stateTag=2;
  static const int32_t statsTag=3;

  int32_t seqNum;
  int connId;

  DeliveryState state;
  DeliveryStatistics stats;

  bool stateFlag;
  bool statsFlag;
};

}
}
}
}
#endif
