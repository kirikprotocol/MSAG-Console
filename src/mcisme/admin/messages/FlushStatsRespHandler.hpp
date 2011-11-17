#ifndef __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_FLUSHSTATSRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_MCISME_ADMIN_MESSAGES_FLUSHSTATSRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) FlushStatsResp version 1.0"



namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class FlushStatsResp:public Response{
public:
  FlushStatsResp()
  {
    Clear();
  }
  void Clear()
  {
    Response::Clear();
  }
 
  static int32_t messageGetTag()
  {
    return 3;
  }

  static std::string messageGetName()
  {
    return "FlushStatsResp";
  }

  std::string toString()const
  {
    std::string rv;
    rv+=Response::toString();
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    rv+=Response::length<DataStream>();
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    Response::serialize(ds);
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
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
    //  throw protogen::framework::IncompatibleVersionException("FlushStatsResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("FlushStatsResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;




};

}
}
}
}
#endif
