#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_APPLYMAPLIMITS_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_APPLYMAPLIMITS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) ApplyMapLimits version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class ApplyMapLimits{
public:
  ApplyMapLimits()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
  }
 
  static int32_t messageGetTag()
  {
    return 6;
  }

  static std::string messageGetName()
  {
    return "ApplyMapLimits";
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
  int32_t length()const
  {
    int32_t rv=0;
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("ApplyMapLimits");
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
          //  throw protogen::framework::UnexpectedTag("ApplyMapLimits",tag);
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

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;


  int32_t seqNum;


};

}
}
}
}
#endif
