#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERIESLISTNEXTRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_GETDELIVERIESLISTNEXTRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryListInfo.hpp"


#ident "@(#) GetDeliveriesListNextResp version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class GetDeliveriesListNextResp{
public:
  GetDeliveriesListNextResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    moreDeliveriesFlag=false;
    infoFlag=false;
    info.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 53;
  }

  static std::string messageGetName()
  {
    return "GetDeliveriesListNextResp";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(moreDeliveriesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="moreDeliveries=";
      rv+=moreDeliveries?"true":"false";
    }
    if(infoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="info=";
      rv+="[";
      bool first=true;
      for(std::vector<DeliveryListInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
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
    if(moreDeliveriesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(moreDeliveries);
    }
    if(infoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<DeliveryListInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  bool getMoreDeliveries()const
  {
    if(!moreDeliveriesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("moreDeliveries");
    }
    return moreDeliveries;
  }
  void setMoreDeliveries(bool argValue)
  {
    moreDeliveries=argValue;
    moreDeliveriesFlag=true;
  }
  bool& getMoreDeliveriesRef()
  {
    moreDeliveriesFlag=true;
    return moreDeliveries;
  }
  bool hasMoreDeliveries()const
  {
    return moreDeliveriesFlag;
  }
  const std::vector<DeliveryListInfo>& getInfo()const
  {
    if(!infoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("info");
    }
    return info;
  }
  void setInfo(const std::vector<DeliveryListInfo>& argValue)
  {
    info=argValue;
    infoFlag=true;
  }
  std::vector<DeliveryListInfo>& getInfoRef()
  {
    infoFlag=true;
    return info;
  }
  bool hasInfo()const
  {
    return infoFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!moreDeliveriesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreDeliveries");
    }
    if(!infoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("info");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(moreDeliveriesTag);
    ds.writeBoolLV(moreDeliveries); 
    ds.writeTag(infoTag);
    typename DataStream::LengthType len=0;
    for(std::vector<DeliveryListInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<DeliveryListInfo>::const_iterator it=info.begin(),end=info.end();it!=end;++it)
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
    //  throw protogen::framework::IncompatibleVersionException("GetDeliveriesListNextResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case moreDeliveriesTag:
        {
          if(moreDeliveriesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("moreDeliveries");
          }
          moreDeliveries=ds.readBoolLV();
          moreDeliveriesFlag=true;
        }break;
        case infoTag:
        {
          if(infoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("info");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            info.push_back(DeliveryListInfo());
            info.back().deserialize(ds);
            rd+=info.back().length<DataStream>();
          }
          infoFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetDeliveriesListNextResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!moreDeliveriesFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("moreDeliveries");
    }
    if(!infoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("info");
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

  static const int32_t moreDeliveriesTag=1;
  static const int32_t infoTag=2;

  int32_t seqNum;
  int connId;

  bool moreDeliveries;
  std::vector<DeliveryListInfo> info;

  bool moreDeliveriesFlag;
  bool infoFlag;
};

}
}
}
}
#endif
