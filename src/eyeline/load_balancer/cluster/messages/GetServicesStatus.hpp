#ifndef __GENERATED_MESSAGE_EYELINE_LOAD_BALANCER_CLUSTER_MESSAGES_GETSERVICESSTATUS_HPP__
#define __GENERATED_MESSAGE_EYELINE_LOAD_BALANCER_CLUSTER_MESSAGES_GETSERVICESSTATUS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) GetServicesStatus version 1.0"



namespace eyeline{
namespace load_balancer{
namespace cluster{
namespace messages{

typedef std::vector<std::string> string_list;

class GetServicesStatus{
public:
  GetServicesStatus()
  {
    Clear();
  }
  void Clear()
  {
 
    seqNum=0;
 
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(DataStream::endOfMessage_tag);
  }

  template <class DataStream>
  void deserialize(DataStream& ds)
  {
    Clear();
    bool endOfMessage=false;
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("GetServicesStatus");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("GetServicesStatus",tag);
          //}
          ds.skip(ds.readLength());
      }
    }

  }

  uint32_t getSeqNum()const
  {
    return seqNum;
  }
 
  void setSeqNum(uint32_t value)
  {
    seqNum=value;
  }

protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;


  uint32_t seqNum;
 


};

}
}
}
}
#endif
