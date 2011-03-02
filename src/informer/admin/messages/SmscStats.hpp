#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SMSCSTATS_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_ADMIN_MESSAGES_SMSCSTATS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) SmscStats version 1.0"



namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class SmscStats{
public:
  SmscStats()
  {
    Clear();
  }
  void Clear()
  {
    smscIdFlag=false;
    liveTimeFlag=false;
    numberOfRegionsFlag=false;
    maxBandwidthFlag=false;
    averagingPeriodFlag=false;
    currentLoadFlag=false;
    pendingResponsesFlag=false;
    pendingReceiptsFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "SmscStats";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(smscIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smscId=";
      rv+=smscId;
    }
    if(liveTimeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="liveTime=";
      sprintf(buf,"%d",liveTime);
      rv+=buf;
    }
    if(numberOfRegionsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="numberOfRegions=";
      sprintf(buf,"%d",numberOfRegions);
      rv+=buf;
    }
    if(maxBandwidthFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="maxBandwidth=";
      sprintf(buf,"%d",maxBandwidth);
      rv+=buf;
    }
    if(averagingPeriodFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="averagingPeriod=";
      sprintf(buf,"%d",averagingPeriod);
      rv+=buf;
    }
    if(currentLoadFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="currentLoad=";
      sprintf(buf,"%d",currentLoad);
      rv+=buf;
    }
    if(pendingResponsesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="pendingResponses=";
      sprintf(buf,"%d",pendingResponses);
      rv+=buf;
    }
    if(pendingReceiptsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="pendingReceipts=";
      sprintf(buf,"%d",pendingReceipts);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(smscIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(smscId);
    }
    if(liveTimeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(liveTime);
    }
    if(numberOfRegionsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(numberOfRegions);
    }
    if(maxBandwidthFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(maxBandwidth);
    }
    if(averagingPeriodFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(averagingPeriod);
    }
    if(currentLoadFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(currentLoad);
    }
    if(pendingResponsesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(pendingResponses);
    }
    if(pendingReceiptsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(pendingReceipts);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getSmscId()const
  {
    if(!smscIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("smscId");
    }
    return smscId;
  }
  void setSmscId(const std::string& argValue)
  {
    smscId=argValue;
    smscIdFlag=true;
  }
  std::string& getSmscIdRef()
  {
    smscIdFlag=true;
    return smscId;
  }
  bool hasSmscId()const
  {
    return smscIdFlag;
  }
  int32_t getLiveTime()const
  {
    if(!liveTimeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("liveTime");
    }
    return liveTime;
  }
  void setLiveTime(int32_t argValue)
  {
    liveTime=argValue;
    liveTimeFlag=true;
  }
  int32_t& getLiveTimeRef()
  {
    liveTimeFlag=true;
    return liveTime;
  }
  bool hasLiveTime()const
  {
    return liveTimeFlag;
  }
  int32_t getNumberOfRegions()const
  {
    if(!numberOfRegionsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("numberOfRegions");
    }
    return numberOfRegions;
  }
  void setNumberOfRegions(int32_t argValue)
  {
    numberOfRegions=argValue;
    numberOfRegionsFlag=true;
  }
  int32_t& getNumberOfRegionsRef()
  {
    numberOfRegionsFlag=true;
    return numberOfRegions;
  }
  bool hasNumberOfRegions()const
  {
    return numberOfRegionsFlag;
  }
  int32_t getMaxBandwidth()const
  {
    if(!maxBandwidthFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("maxBandwidth");
    }
    return maxBandwidth;
  }
  void setMaxBandwidth(int32_t argValue)
  {
    maxBandwidth=argValue;
    maxBandwidthFlag=true;
  }
  int32_t& getMaxBandwidthRef()
  {
    maxBandwidthFlag=true;
    return maxBandwidth;
  }
  bool hasMaxBandwidth()const
  {
    return maxBandwidthFlag;
  }
  int32_t getAveragingPeriod()const
  {
    if(!averagingPeriodFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("averagingPeriod");
    }
    return averagingPeriod;
  }
  void setAveragingPeriod(int32_t argValue)
  {
    averagingPeriod=argValue;
    averagingPeriodFlag=true;
  }
  int32_t& getAveragingPeriodRef()
  {
    averagingPeriodFlag=true;
    return averagingPeriod;
  }
  bool hasAveragingPeriod()const
  {
    return averagingPeriodFlag;
  }
  int32_t getCurrentLoad()const
  {
    if(!currentLoadFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("currentLoad");
    }
    return currentLoad;
  }
  void setCurrentLoad(int32_t argValue)
  {
    currentLoad=argValue;
    currentLoadFlag=true;
  }
  int32_t& getCurrentLoadRef()
  {
    currentLoadFlag=true;
    return currentLoad;
  }
  bool hasCurrentLoad()const
  {
    return currentLoadFlag;
  }
  int32_t getPendingResponses()const
  {
    if(!pendingResponsesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("pendingResponses");
    }
    return pendingResponses;
  }
  void setPendingResponses(int32_t argValue)
  {
    pendingResponses=argValue;
    pendingResponsesFlag=true;
  }
  int32_t& getPendingResponsesRef()
  {
    pendingResponsesFlag=true;
    return pendingResponses;
  }
  bool hasPendingResponses()const
  {
    return pendingResponsesFlag;
  }
  int32_t getPendingReceipts()const
  {
    if(!pendingReceiptsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("pendingReceipts");
    }
    return pendingReceipts;
  }
  void setPendingReceipts(int32_t argValue)
  {
    pendingReceipts=argValue;
    pendingReceiptsFlag=true;
  }
  int32_t& getPendingReceiptsRef()
  {
    pendingReceiptsFlag=true;
    return pendingReceipts;
  }
  bool hasPendingReceipts()const
  {
    return pendingReceiptsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!smscIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smscId");
    }
    if(!liveTimeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("liveTime");
    }
    if(!numberOfRegionsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("numberOfRegions");
    }
    if(!maxBandwidthFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("maxBandwidth");
    }
    if(!averagingPeriodFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("averagingPeriod");
    }
    if(!currentLoadFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentLoad");
    }
    if(!pendingResponsesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("pendingResponses");
    }
    if(!pendingReceiptsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("pendingReceipts");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(smscIdTag);
    ds.writeStrLV(smscId); 
    ds.writeTag(liveTimeTag);
    ds.writeInt32LV(liveTime); 
    ds.writeTag(numberOfRegionsTag);
    ds.writeInt32LV(numberOfRegions); 
    ds.writeTag(maxBandwidthTag);
    ds.writeInt32LV(maxBandwidth); 
    ds.writeTag(averagingPeriodTag);
    ds.writeInt32LV(averagingPeriod); 
    ds.writeTag(currentLoadTag);
    ds.writeInt32LV(currentLoad); 
    ds.writeTag(pendingResponsesTag);
    ds.writeInt32LV(pendingResponses); 
    ds.writeTag(pendingReceiptsTag);
    ds.writeInt32LV(pendingReceipts); 
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
    //  throw protogen::framework::IncompatibleVersionException("SmscStats");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case smscIdTag:
        {
          if(smscIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("smscId");
          }
          smscId=ds.readStrLV();
          smscIdFlag=true;
        }break;
        case liveTimeTag:
        {
          if(liveTimeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("liveTime");
          }
          liveTime=ds.readInt32LV();
          liveTimeFlag=true;
        }break;
        case numberOfRegionsTag:
        {
          if(numberOfRegionsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("numberOfRegions");
          }
          numberOfRegions=ds.readInt32LV();
          numberOfRegionsFlag=true;
        }break;
        case maxBandwidthTag:
        {
          if(maxBandwidthFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("maxBandwidth");
          }
          maxBandwidth=ds.readInt32LV();
          maxBandwidthFlag=true;
        }break;
        case averagingPeriodTag:
        {
          if(averagingPeriodFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("averagingPeriod");
          }
          averagingPeriod=ds.readInt32LV();
          averagingPeriodFlag=true;
        }break;
        case currentLoadTag:
        {
          if(currentLoadFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("currentLoad");
          }
          currentLoad=ds.readInt32LV();
          currentLoadFlag=true;
        }break;
        case pendingResponsesTag:
        {
          if(pendingResponsesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("pendingResponses");
          }
          pendingResponses=ds.readInt32LV();
          pendingResponsesFlag=true;
        }break;
        case pendingReceiptsTag:
        {
          if(pendingReceiptsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("pendingReceipts");
          }
          pendingReceipts=ds.readInt32LV();
          pendingReceiptsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("SmscStats",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!smscIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("smscId");
    }
    if(!liveTimeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("liveTime");
    }
    if(!numberOfRegionsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("numberOfRegions");
    }
    if(!maxBandwidthFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("maxBandwidth");
    }
    if(!averagingPeriodFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("averagingPeriod");
    }
    if(!currentLoadFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("currentLoad");
    }
    if(!pendingResponsesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("pendingResponses");
    }
    if(!pendingReceiptsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("pendingReceipts");
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

  static const int32_t smscIdTag=1;
  static const int32_t liveTimeTag=2;
  static const int32_t numberOfRegionsTag=3;
  static const int32_t maxBandwidthTag=4;
  static const int32_t averagingPeriodTag=5;
  static const int32_t currentLoadTag=6;
  static const int32_t pendingResponsesTag=7;
  static const int32_t pendingReceiptsTag=8;

  int connId;

  std::string smscId;
  int32_t liveTime;
  int32_t numberOfRegions;
  int32_t maxBandwidth;
  int32_t averagingPeriod;
  int32_t currentLoad;
  int32_t pendingResponses;
  int32_t pendingReceipts;

  bool smscIdFlag;
  bool liveTimeFlag;
  bool numberOfRegionsFlag;
  bool maxBandwidthFlag;
  bool averagingPeriodFlag;
  bool currentLoadFlag;
  bool pendingResponsesFlag;
  bool pendingReceiptsFlag;
};

}
}
}
}
#endif
