#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ALIASDEL_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ALIASDEL_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) AliasDel version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class AliasDel{
public:
  AliasDel()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    aliasFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 42;
  }

  static std::string messageGetName()
  {
    return "AliasDel";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(aliasFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="alias=";
      rv+=alias;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(aliasFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(alias);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getAlias()const
  {
    if(!aliasFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("alias");
    }
    return alias;
  }
  void setAlias(const std::string& argValue)
  {
    alias=argValue;
    aliasFlag=true;
  }
  std::string& getAliasRef()
  {
    aliasFlag=true;
    return alias;
  }
  bool hasAlias()const
  {
    return aliasFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!aliasFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("alias");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(aliasTag);
    ds.writeStrLV(alias); 
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
    //  throw protogen::framework::IncompatibleVersionException("AliasDel");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case aliasTag:
        {
          if(aliasFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("alias");
          }
          alias=ds.readStrLV();
          aliasFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AliasDel",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!aliasFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("alias");
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

  static const int32_t aliasTag=1;

  int32_t seqNum;

  std::string alias;

  bool aliasFlag;
};

}
}
}
}
#endif
