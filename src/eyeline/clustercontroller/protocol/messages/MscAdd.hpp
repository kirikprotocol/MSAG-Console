#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_MSCADD_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_MSCADD_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) MscAdd version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class MscAdd{
public:
  MscAdd()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    mscFlag=false;
  }
 
  static int32_t getTag()
  {
    return 16;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(mscFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="msc=";
      rv+=msc;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(mscFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(msc);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getMsc()const
  {
    if(!mscFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("msc");
    }
    return msc;
  }
  void setMsc(const std::string& argValue)
  {
    msc=argValue;
    mscFlag=true;
  }
  std::string& getMscRef()
  {
    mscFlag=true;
    return msc;
  }
  bool hasMsc()const
  {
    return mscFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!mscFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("msc");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(mscTag);
    ds.writeStrLV(msc);
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
    //  throw protogen::framework::IncompatibleVersionException("MscAdd");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case mscTag:
        {
          if(mscFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("msc");
          }
          msc=ds.readStrLV();
          mscFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("MscAdd",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!mscFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("msc");
    }

  }

  int32_t getSeqNum()const
  {
    return seqNum;
  }

  void setSeqNum(int32_t argValue)
  {
    seqNum=argValue;
  }

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t mscTag=1;

  int32_t seqNum;

  std::string msc;

  bool mscFlag;
};

}
}
}
}
#endif
