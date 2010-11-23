#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMDELETEGROUP_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CGMDELETEGROUP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CgmDeleteGroup version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class CgmDeleteGroup{
public:
  CgmDeleteGroup()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    idFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 34;
  }

  static std::string messageGetName()
  {
    return "CgmDeleteGroup";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(idFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="id=";
      sprintf(buf,"%d",id);
      rv+=buf;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(idFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(id);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int32_t getId()const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(int32_t argValue)
  {
    id=argValue;
    idFlag=true;
  }
  int32_t& getIdRef()
  {
    idFlag=true;
    return id;
  }
  bool hasId()const
  {
    return idFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeInt32LV(id); 
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
    //  throw protogen::framework::IncompatibleVersionException("CgmDeleteGroup");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readInt32LV();
          idFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CgmDeleteGroup",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
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

  static const int32_t idTag=1;

  int32_t seqNum;

  int32_t id;

  bool idFlag;
};

}
}
}
}
}
#endif
