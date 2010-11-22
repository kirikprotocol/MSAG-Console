#ifndef __GENERATED_MESSAGE_SMSC_MCISME_MCAIA_BUSYRESPONSE_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_MCAIA_BUSYRESPONSE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "BusyRequest.hpp"
#include "Status.hpp"


#ident "@(#) BusyResponse version 1.0"



namespace smsc{
namespace mcisme{
namespace mcaia{


class BusyResponse:public BusyRequest{
public:
  BusyResponse()
  {
    Clear();
  }
  void Clear()
  {
    BusyRequest::Clear();
    statusFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 2;
  }

  static std::string messageGetName()
  {
    return "BusyResponse";
  }

  std::string toString()const
  {
    std::string rv;
    rv+=BusyRequest::toString();
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+=Status::getNameByValue(status);
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    rv+=BusyRequest::length<DataStream>();
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status.getValue());
 
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const Status& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const Status& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  Status& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    BusyRequest::serialize(ds);
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeByteLV(status.getValue());
 
    ds.writeTag(DataStream::endOfMessage_tag);
  }

  template <class DataStream>
  void deserialize(DataStream& ds)
  {
    Clear();
    BusyRequest::deserialize(ds);
    bool endOfMessage=false;
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("BusyResponse");
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
          status=ds.readByteLV();
          statusFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("BusyResponse",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t statusTag=4;


  Status status;

  bool statusFlag;
};

}
}
}
#endif
