#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETUSERSTATSRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETUSERSTATSRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) GetUserStatsResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetUserStatsResp{
public:
  GetUserStatsResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    currentConnectionsCountFlag=false;
    currentActiveDeliveriesCountFlag=false;
    currentPausedDeliveriesCountFlag=false;
    currentPlannedDeliveriesCountFlag=false;
    currentFinishedDeliveriesCountFlag=false;
    currentCancelledDeliveriesCountFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 13;
  }

  static std::string messageGetName()
  {
    return "GetUserStatsResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(currentConnectionsCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentConnectionsCount=";
      sprintf(buf,"%d",(int)currentConnectionsCount);
      rv+=buf;
    }
    if(currentActiveDeliveriesCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentActiveDeliveriesCount=";
      sprintf(buf,"%d",currentActiveDeliveriesCount);
      rv+=buf;
    }
    if(currentPausedDeliveriesCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentPausedDeliveriesCount=";
      sprintf(buf,"%d",currentPausedDeliveriesCount);
      rv+=buf;
    }
    if(currentPlannedDeliveriesCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentPlannedDeliveriesCount=";
      sprintf(buf,"%d",currentPlannedDeliveriesCount);
      rv+=buf;
    }
    if(currentFinishedDeliveriesCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentFinishedDeliveriesCount=";
      sprintf(buf,"%d",currentFinishedDeliveriesCount);
      rv+=buf;
    }
    if(currentCancelledDeliveriesCountFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentCancelledDeliveriesCount=";
      sprintf(buf,"%d",currentCancelledDeliveriesCount);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(currentConnectionsCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentConnectionsCount);
    }
    if(currentActiveDeliveriesCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentActiveDeliveriesCount);
    }
    if(currentPausedDeliveriesCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentPausedDeliveriesCount);
    }
    if(currentPlannedDeliveriesCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentPlannedDeliveriesCount);
    }
    if(currentFinishedDeliveriesCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentFinishedDeliveriesCount);
    }
    if(currentCancelledDeliveriesCountFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentCancelledDeliveriesCount);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int16_t getCurrentConnectionsCount()const
  {
    if(!currentConnectionsCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentConnectionsCount");
    }
    return currentConnectionsCount;
  }
  void setCurrentConnectionsCount(int16_t argValue)
  {
    currentConnectionsCount=argValue;
    currentConnectionsCountFlag=true;
  }
  int16_t& getCurrentConnectionsCountRef()
  {
    currentConnectionsCountFlag=true;
    return currentConnectionsCount;
  }
  bool hasCurrentConnectionsCount()const
  {
    return currentConnectionsCountFlag;
  }
  int32_t getCurrentActiveDeliveriesCount()const
  {
    if(!currentActiveDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentActiveDeliveriesCount");
    }
    return currentActiveDeliveriesCount;
  }
  void setCurrentActiveDeliveriesCount(int32_t argValue)
  {
    currentActiveDeliveriesCount=argValue;
    currentActiveDeliveriesCountFlag=true;
  }
  int32_t& getCurrentActiveDeliveriesCountRef()
  {
    currentActiveDeliveriesCountFlag=true;
    return currentActiveDeliveriesCount;
  }
  bool hasCurrentActiveDeliveriesCount()const
  {
    return currentActiveDeliveriesCountFlag;
  }
  int32_t getCurrentPausedDeliveriesCount()const
  {
    if(!currentPausedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentPausedDeliveriesCount");
    }
    return currentPausedDeliveriesCount;
  }
  void setCurrentPausedDeliveriesCount(int32_t argValue)
  {
    currentPausedDeliveriesCount=argValue;
    currentPausedDeliveriesCountFlag=true;
  }
  int32_t& getCurrentPausedDeliveriesCountRef()
  {
    currentPausedDeliveriesCountFlag=true;
    return currentPausedDeliveriesCount;
  }
  bool hasCurrentPausedDeliveriesCount()const
  {
    return currentPausedDeliveriesCountFlag;
  }
  int32_t getCurrentPlannedDeliveriesCount()const
  {
    if(!currentPlannedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentPlannedDeliveriesCount");
    }
    return currentPlannedDeliveriesCount;
  }
  void setCurrentPlannedDeliveriesCount(int32_t argValue)
  {
    currentPlannedDeliveriesCount=argValue;
    currentPlannedDeliveriesCountFlag=true;
  }
  int32_t& getCurrentPlannedDeliveriesCountRef()
  {
    currentPlannedDeliveriesCountFlag=true;
    return currentPlannedDeliveriesCount;
  }
  bool hasCurrentPlannedDeliveriesCount()const
  {
    return currentPlannedDeliveriesCountFlag;
  }
  int32_t getCurrentFinishedDeliveriesCount()const
  {
    if(!currentFinishedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentFinishedDeliveriesCount");
    }
    return currentFinishedDeliveriesCount;
  }
  void setCurrentFinishedDeliveriesCount(int32_t argValue)
  {
    currentFinishedDeliveriesCount=argValue;
    currentFinishedDeliveriesCountFlag=true;
  }
  int32_t& getCurrentFinishedDeliveriesCountRef()
  {
    currentFinishedDeliveriesCountFlag=true;
    return currentFinishedDeliveriesCount;
  }
  bool hasCurrentFinishedDeliveriesCount()const
  {
    return currentFinishedDeliveriesCountFlag;
  }
  int32_t getCurrentCancelledDeliveriesCount()const
  {
    if(!currentCancelledDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentCancelledDeliveriesCount");
    }
    return currentCancelledDeliveriesCount;
  }
  void setCurrentCancelledDeliveriesCount(int32_t argValue)
  {
    currentCancelledDeliveriesCount=argValue;
    currentCancelledDeliveriesCountFlag=true;
  }
  int32_t& getCurrentCancelledDeliveriesCountRef()
  {
    currentCancelledDeliveriesCountFlag=true;
    return currentCancelledDeliveriesCount;
  }
  bool hasCurrentCancelledDeliveriesCount()const
  {
    return currentCancelledDeliveriesCountFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!currentConnectionsCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentConnectionsCount");
    }
    if(!currentActiveDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentActiveDeliveriesCount");
    }
    if(!currentPausedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentPausedDeliveriesCount");
    }
    if(!currentPlannedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentPlannedDeliveriesCount");
    }
    if(!currentFinishedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentFinishedDeliveriesCount");
    }
    if(!currentCancelledDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentCancelledDeliveriesCount");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(currentConnectionsCountTag);
    ds.writeInt16LV(currentConnectionsCount); 
    ds.writeTag(currentActiveDeliveriesCountTag);
    ds.writeInt32LV(currentActiveDeliveriesCount); 
    ds.writeTag(currentPausedDeliveriesCountTag);
    ds.writeInt32LV(currentPausedDeliveriesCount); 
    ds.writeTag(currentPlannedDeliveriesCountTag);
    ds.writeInt32LV(currentPlannedDeliveriesCount); 
    ds.writeTag(currentFinishedDeliveriesCountTag);
    ds.writeInt32LV(currentFinishedDeliveriesCount); 
    ds.writeTag(currentCancelledDeliveriesCountTag);
    ds.writeInt32LV(currentCancelledDeliveriesCount); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetUserStatsResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case currentConnectionsCountTag:
        {
          if(currentConnectionsCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentConnectionsCount");
          }
          currentConnectionsCount=ds.readInt16LV();
          currentConnectionsCountFlag=true;
        }break;
        case currentActiveDeliveriesCountTag:
        {
          if(currentActiveDeliveriesCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentActiveDeliveriesCount");
          }
          currentActiveDeliveriesCount=ds.readInt32LV();
          currentActiveDeliveriesCountFlag=true;
        }break;
        case currentPausedDeliveriesCountTag:
        {
          if(currentPausedDeliveriesCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentPausedDeliveriesCount");
          }
          currentPausedDeliveriesCount=ds.readInt32LV();
          currentPausedDeliveriesCountFlag=true;
        }break;
        case currentPlannedDeliveriesCountTag:
        {
          if(currentPlannedDeliveriesCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentPlannedDeliveriesCount");
          }
          currentPlannedDeliveriesCount=ds.readInt32LV();
          currentPlannedDeliveriesCountFlag=true;
        }break;
        case currentFinishedDeliveriesCountTag:
        {
          if(currentFinishedDeliveriesCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentFinishedDeliveriesCount");
          }
          currentFinishedDeliveriesCount=ds.readInt32LV();
          currentFinishedDeliveriesCountFlag=true;
        }break;
        case currentCancelledDeliveriesCountTag:
        {
          if(currentCancelledDeliveriesCountFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentCancelledDeliveriesCount");
          }
          currentCancelledDeliveriesCount=ds.readInt32LV();
          currentCancelledDeliveriesCountFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetUserStatsResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!currentConnectionsCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentConnectionsCount");
    }
    if(!currentActiveDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentActiveDeliveriesCount");
    }
    if(!currentPausedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentPausedDeliveriesCount");
    }
    if(!currentPlannedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentPlannedDeliveriesCount");
    }
    if(!currentFinishedDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentFinishedDeliveriesCount");
    }
    if(!currentCancelledDeliveriesCountFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentCancelledDeliveriesCount");
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

  static const int32_t currentConnectionsCountTag=1;
  static const int32_t currentActiveDeliveriesCountTag=2;
  static const int32_t currentPausedDeliveriesCountTag=3;
  static const int32_t currentPlannedDeliveriesCountTag=4;
  static const int32_t currentFinishedDeliveriesCountTag=5;
  static const int32_t currentCancelledDeliveriesCountTag=6;

  int32_t seqNum;
  int connId;

  int16_t currentConnectionsCount;
  int32_t currentActiveDeliveriesCount;
  int32_t currentPausedDeliveriesCount;
  int32_t currentPlannedDeliveriesCount;
  int32_t currentFinishedDeliveriesCount;
  int32_t currentCancelledDeliveriesCount;

  bool currentConnectionsCountFlag;
  bool currentActiveDeliveriesCountFlag;
  bool currentPausedDeliveriesCountFlag;
  bool currentPlannedDeliveriesCountFlag;
  bool currentFinishedDeliveriesCountFlag;
  bool currentCancelledDeliveriesCountFlag;
};

}
}
}
}
#endif
