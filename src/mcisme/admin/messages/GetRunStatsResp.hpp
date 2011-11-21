#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETRUNSTATSRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_GETRUNSTATSRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) GetRunStatsResp version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class GetRunStatsResp:public Response{
public:
  GetRunStatsResp()
  {
    Clear();
  }
  void Clear()
  {
    Response::Clear();
    activeTasksFlag=false;
    inQueueSizeFlag=false;
    outQueueSizeFlag=false;
    inSpeedFlag=false;
    outSpeedFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 7;
  }

  static std::string messageGetName()
  {
    return "GetRunStatsResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    rv+=Response::toString();
    if(activeTasksFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="activeTasks=";
      sprintf(buf,"%d",activeTasks);
      rv+=buf;
    }
    if(inQueueSizeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="inQueueSize=";
      sprintf(buf,"%d",inQueueSize);
      rv+=buf;
    }
    if(outQueueSizeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="outQueueSize=";
      sprintf(buf,"%d",outQueueSize);
      rv+=buf;
    }
    if(inSpeedFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="inSpeed=";
      sprintf(buf,"%d",inSpeed);
      rv+=buf;
    }
    if(outSpeedFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="outSpeed=";
      sprintf(buf,"%d",outSpeed);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    rv+=Response::length<DataStream>();
    if(activeTasksFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(activeTasks);
    }
    if(inQueueSizeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(inQueueSize);
    }
    if(outQueueSizeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(outQueueSize);
    }
    if(inSpeedFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(inSpeed);
    }
    if(outSpeedFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(outSpeed);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getActiveTasks()const
  {
    if(!activeTasksFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("activeTasks");
    }
    return activeTasks;
  }
  void setActiveTasks(int32_t argValue)
  {
    activeTasks=argValue;
    activeTasksFlag=true;
  }
  int32_t& getActiveTasksRef()
  {
    activeTasksFlag=true;
    return activeTasks;
  }
  bool hasActiveTasks()const
  {
    return activeTasksFlag;
  }
  int32_t getInQueueSize()const
  {
    if(!inQueueSizeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("inQueueSize");
    }
    return inQueueSize;
  }
  void setInQueueSize(int32_t argValue)
  {
    inQueueSize=argValue;
    inQueueSizeFlag=true;
  }
  int32_t& getInQueueSizeRef()
  {
    inQueueSizeFlag=true;
    return inQueueSize;
  }
  bool hasInQueueSize()const
  {
    return inQueueSizeFlag;
  }
  int32_t getOutQueueSize()const
  {
    if(!outQueueSizeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("outQueueSize");
    }
    return outQueueSize;
  }
  void setOutQueueSize(int32_t argValue)
  {
    outQueueSize=argValue;
    outQueueSizeFlag=true;
  }
  int32_t& getOutQueueSizeRef()
  {
    outQueueSizeFlag=true;
    return outQueueSize;
  }
  bool hasOutQueueSize()const
  {
    return outQueueSizeFlag;
  }
  int32_t getInSpeed()const
  {
    if(!inSpeedFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("inSpeed");
    }
    return inSpeed;
  }
  void setInSpeed(int32_t argValue)
  {
    inSpeed=argValue;
    inSpeedFlag=true;
  }
  int32_t& getInSpeedRef()
  {
    inSpeedFlag=true;
    return inSpeed;
  }
  bool hasInSpeed()const
  {
    return inSpeedFlag;
  }
  int32_t getOutSpeed()const
  {
    if(!outSpeedFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("outSpeed");
    }
    return outSpeed;
  }
  void setOutSpeed(int32_t argValue)
  {
    outSpeed=argValue;
    outSpeedFlag=true;
  }
  int32_t& getOutSpeedRef()
  {
    outSpeedFlag=true;
    return outSpeed;
  }
  bool hasOutSpeed()const
  {
    return outSpeedFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    Response::serialize(ds);
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(activeTasksFlag)
    {
      ds.writeTag(activeTasksTag);
    ds.writeInt32LV(activeTasks); 
    }
    if(inQueueSizeFlag)
    {
      ds.writeTag(inQueueSizeTag);
    ds.writeInt32LV(inQueueSize); 
    }
    if(outQueueSizeFlag)
    {
      ds.writeTag(outQueueSizeTag);
    ds.writeInt32LV(outQueueSize); 
    }
    if(inSpeedFlag)
    {
      ds.writeTag(inSpeedTag);
    ds.writeInt32LV(inSpeed); 
    }
    if(outSpeedFlag)
    {
      ds.writeTag(outSpeedTag);
    ds.writeInt32LV(outSpeed); 
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
    //  throw protogen::framework::IncompatibleVersionException("GetRunStatsResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case activeTasksTag:
        {
          if(activeTasksFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("activeTasks");
          }
          activeTasks=ds.readInt32LV();
          activeTasksFlag=true;
        }break;
        case inQueueSizeTag:
        {
          if(inQueueSizeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("inQueueSize");
          }
          inQueueSize=ds.readInt32LV();
          inQueueSizeFlag=true;
        }break;
        case outQueueSizeTag:
        {
          if(outQueueSizeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("outQueueSize");
          }
          outQueueSize=ds.readInt32LV();
          outQueueSizeFlag=true;
        }break;
        case inSpeedTag:
        {
          if(inSpeedFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("inSpeed");
          }
          inSpeed=ds.readInt32LV();
          inSpeedFlag=true;
        }break;
        case outSpeedTag:
        {
          if(outSpeedFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("outSpeed");
          }
          outSpeed=ds.readInt32LV();
          outSpeedFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetRunStatsResp",tag);
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

  static const int32_t activeTasksTag=1;
  static const int32_t inQueueSizeTag=2;
  static const int32_t outQueueSizeTag=3;
  static const int32_t inSpeedTag=4;
  static const int32_t outSpeedTag=5;

  int connId;

  int32_t activeTasks;
  int32_t inQueueSize;
  int32_t outQueueSize;
  int32_t inSpeed;
  int32_t outSpeed;

  bool activeTasksFlag;
  bool inQueueSizeFlag;
  bool outQueueSizeFlag;
  bool inSpeedFlag;
  bool outSpeedFlag;
};

}
}
}
}
#endif
