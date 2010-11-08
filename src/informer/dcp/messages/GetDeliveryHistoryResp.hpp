#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERYHISTORYRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERYHISTORYRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryHistoryItem.hpp"


#ident "@(#) GetDeliveryHistoryResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetDeliveryHistoryResp{
public:
  GetDeliveryHistoryResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    historyFlag=false;
    history.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 59;
  }

  static std::string messageGetName()
  {
    return "GetDeliveryHistoryResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(historyFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="history=";
      rv+="[";
      bool first=true;
      for(std::vector<DeliveryHistoryItem>::const_iterator it=history.begin(),end=history.end();it!=end;++it)
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
    if(historyFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<DeliveryHistoryItem>::const_iterator it=history.begin(),end=history.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<DeliveryHistoryItem>& getHistory()const
  {
    if(!historyFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("history");
    }
    return history;
  }
  void setHistory(const std::vector<DeliveryHistoryItem>& argValue)
  {
    history=argValue;
    historyFlag=true;
  }
  std::vector<DeliveryHistoryItem>& getHistoryRef()
  {
    historyFlag=true;
    return history;
  }
  bool hasHistory()const
  {
    return historyFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!historyFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("history");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(historyTag);
    typename DataStream::LengthType len=0;
    for(std::vector<DeliveryHistoryItem>::const_iterator it=history.begin(),end=history.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<DeliveryHistoryItem>::const_iterator it=history.begin(),end=history.end();it!=end;++it)
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
    //  throw protogen::framework::IncompatibleVersionException("GetDeliveryHistoryResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case historyTag:
        {
          if(historyFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("history");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            history.push_back(DeliveryHistoryItem());
            history.back().deserialize(ds);
            rd+=history.back().length<DataStream>();
          }
          historyFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetDeliveryHistoryResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!historyFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("history");
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

  static const int32_t historyTag=1;

  int32_t seqNum;
  int connId;

  std::vector<DeliveryHistoryItem> history;

  bool historyFlag;
};

}
}
}
}
#endif
