#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIES_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_COUNTDELIVERIES_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryStatus.hpp"


#ident "@(#) CountDeliveries version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class CountDeliveries{
public:
  CountDeliveries()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    userIdFilterFlag=false;
    userIdFilter.clear();
    nameFilterFlag=false;
    nameFilter.clear();
    statusFilterFlag=false;
    statusFilter.clear();
    startDateFromFlag=false;
    startDateToFlag=false;
    endDateFromFlag=false;
    endDateToFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 54;
  }

  static std::string messageGetName()
  {
    return "CountDeliveries";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(userIdFilterFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="userIdFilter=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=userIdFilter.begin(),end=userIdFilter.end();it!=end;++it)
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
    if(nameFilterFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="nameFilter=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=nameFilter.begin(),end=nameFilter.end();it!=end;++it)
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
    if(statusFilterFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="statusFilter=";
      rv+="[";
      bool first=true;
      for(std::vector<DeliveryStatus>::const_iterator it=statusFilter.begin(),end=statusFilter.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=DeliveryStatus::getNameByValue(*it);
      }
    }
    if(startDateFromFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="startDateFrom=";
      rv+=startDateFrom;
    }
    if(startDateToFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="startDateTo=";
      rv+=startDateTo;
    }
    if(endDateFromFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="endDateFrom=";
      rv+=endDateFrom;
    }
    if(endDateToFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="endDateTo=";
      rv+=endDateTo;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(userIdFilterFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userIdFilter);
    }
    if(nameFilterFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(nameFilter);
    }
    if(statusFilterFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(statusFilter);
 
    }
    if(startDateFromFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(startDateFrom);
    }
    if(startDateToFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(startDateTo);
    }
    if(endDateFromFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(endDateFrom);
    }
    if(endDateToFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(endDateTo);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<std::string>& getUserIdFilter()const
  {
    if(!userIdFilterFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("userIdFilter");
    }
    return userIdFilter;
  }
  void setUserIdFilter(const std::vector<std::string>& argValue)
  {
    userIdFilter=argValue;
    userIdFilterFlag=true;
  }
  std::vector<std::string>& getUserIdFilterRef()
  {
    userIdFilterFlag=true;
    return userIdFilter;
  }
  bool hasUserIdFilter()const
  {
    return userIdFilterFlag;
  }
  const std::vector<std::string>& getNameFilter()const
  {
    if(!nameFilterFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("nameFilter");
    }
    return nameFilter;
  }
  void setNameFilter(const std::vector<std::string>& argValue)
  {
    nameFilter=argValue;
    nameFilterFlag=true;
  }
  std::vector<std::string>& getNameFilterRef()
  {
    nameFilterFlag=true;
    return nameFilter;
  }
  bool hasNameFilter()const
  {
    return nameFilterFlag;
  }
  const std::vector<DeliveryStatus>& getStatusFilter()const
  {
    if(!statusFilterFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("statusFilter");
    }
    return statusFilter;
  }
  void setStatusFilter(const std::vector<DeliveryStatus>& argValue)
  {
    statusFilter=argValue;
    statusFilterFlag=true;
  }
  std::vector<DeliveryStatus>& getStatusFilterRef()
  {
    statusFilterFlag=true;
    return statusFilter;
  }
  bool hasStatusFilter()const
  {
    return statusFilterFlag;
  }
  const std::string& getStartDateFrom()const
  {
    if(!startDateFromFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("startDateFrom");
    }
    return startDateFrom;
  }
  void setStartDateFrom(const std::string& argValue)
  {
    startDateFrom=argValue;
    startDateFromFlag=true;
  }
  std::string& getStartDateFromRef()
  {
    startDateFromFlag=true;
    return startDateFrom;
  }
  bool hasStartDateFrom()const
  {
    return startDateFromFlag;
  }
  const std::string& getStartDateTo()const
  {
    if(!startDateToFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("startDateTo");
    }
    return startDateTo;
  }
  void setStartDateTo(const std::string& argValue)
  {
    startDateTo=argValue;
    startDateToFlag=true;
  }
  std::string& getStartDateToRef()
  {
    startDateToFlag=true;
    return startDateTo;
  }
  bool hasStartDateTo()const
  {
    return startDateToFlag;
  }
  const std::string& getEndDateFrom()const
  {
    if(!endDateFromFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("endDateFrom");
    }
    return endDateFrom;
  }
  void setEndDateFrom(const std::string& argValue)
  {
    endDateFrom=argValue;
    endDateFromFlag=true;
  }
  std::string& getEndDateFromRef()
  {
    endDateFromFlag=true;
    return endDateFrom;
  }
  bool hasEndDateFrom()const
  {
    return endDateFromFlag;
  }
  const std::string& getEndDateTo()const
  {
    if(!endDateToFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("endDateTo");
    }
    return endDateTo;
  }
  void setEndDateTo(const std::string& argValue)
  {
    endDateTo=argValue;
    endDateToFlag=true;
  }
  std::string& getEndDateToRef()
  {
    endDateToFlag=true;
    return endDateTo;
  }
  bool hasEndDateTo()const
  {
    return endDateToFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    if(userIdFilterFlag)
    {
      ds.writeTag(userIdFilterTag);
    ds.writeLength(DataStream::fieldSize(userIdFilter));
    for(std::vector<std::string>::const_iterator it=userIdFilter.begin(),end=userIdFilter.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    }
    if(nameFilterFlag)
    {
      ds.writeTag(nameFilterTag);
    ds.writeLength(DataStream::fieldSize(nameFilter));
    for(std::vector<std::string>::const_iterator it=nameFilter.begin(),end=nameFilter.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    }
    if(statusFilterFlag)
    {
      ds.writeTag(statusFilterTag);
    ds.writeLength(DataStream::fieldSize(statusFilter));
    for(std::vector<DeliveryStatus>::const_iterator it=statusFilter.begin(),end=statusFilter.end();it!=end;++it)
    {
      ds.writeByte(it->getValue());
    }
 
    }
    if(startDateFromFlag)
    {
      ds.writeTag(startDateFromTag);
    ds.writeStrLV(startDateFrom); 
    }
    if(startDateToFlag)
    {
      ds.writeTag(startDateToTag);
    ds.writeStrLV(startDateTo); 
    }
    if(endDateFromFlag)
    {
      ds.writeTag(endDateFromTag);
    ds.writeStrLV(endDateFrom); 
    }
    if(endDateToFlag)
    {
      ds.writeTag(endDateToTag);
    ds.writeStrLV(endDateTo); 
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
    //  throw protogen::framework::IncompatibleVersionException("CountDeliveries");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case userIdFilterTag:
        {
          if(userIdFilterFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("userIdFilter");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            userIdFilter.push_back(ds.readStr());
            rd+=DataStream::fieldSize(userIdFilter.back());
            rd+=DataStream::lengthTypeSize;
          }
          userIdFilterFlag=true;
        }break;
        case nameFilterTag:
        {
          if(nameFilterFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("nameFilter");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            nameFilter.push_back(ds.readStr());
            rd+=DataStream::fieldSize(nameFilter.back());
            rd+=DataStream::lengthTypeSize;
          }
          nameFilterFlag=true;
        }break;
        case statusFilterTag:
        {
          if(statusFilterFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("statusFilter");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            statusFilter.push_back(ds.readByte());
            rd+=DataStream::fieldSize(statusFilter.back());
          }
          statusFilterFlag=true;
        }break;
        case startDateFromTag:
        {
          if(startDateFromFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("startDateFrom");
          }
          startDateFrom=ds.readStrLV();
          startDateFromFlag=true;
        }break;
        case startDateToTag:
        {
          if(startDateToFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("startDateTo");
          }
          startDateTo=ds.readStrLV();
          startDateToFlag=true;
        }break;
        case endDateFromTag:
        {
          if(endDateFromFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("endDateFrom");
          }
          endDateFrom=ds.readStrLV();
          endDateFromFlag=true;
        }break;
        case endDateToTag:
        {
          if(endDateToFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("endDateTo");
          }
          endDateTo=ds.readStrLV();
          endDateToFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CountDeliveries",tag);
          //}
          ds.skip(ds.readLength());
      }
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

  static const int32_t userIdFilterTag=1;
  static const int32_t nameFilterTag=2;
  static const int32_t statusFilterTag=3;
  static const int32_t startDateFromTag=4;
  static const int32_t startDateToTag=5;
  static const int32_t endDateFromTag=6;
  static const int32_t endDateToTag=7;

  int32_t seqNum;
  int connId;

  std::vector<std::string> userIdFilter;
  std::vector<std::string> nameFilter;
  std::vector<DeliveryStatus> statusFilter;
  std::string startDateFrom;
  std::string startDateTo;
  std::string endDateFrom;
  std::string endDateTo;

  bool userIdFilterFlag;
  bool nameFilterFlag;
  bool statusFilterFlag;
  bool startDateFromFlag;
  bool startDateToFlag;
  bool endDateFromFlag;
  bool endDateToFlag;
};

}
}
}
}
#endif
