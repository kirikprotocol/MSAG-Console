#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_UPDATEPROFILEABNT_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_UPDATEPROFILEABNT_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Profile.hpp"


#ident "@(#) UpdateProfileAbnt version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class UpdateProfileAbnt{
public:
  UpdateProfileAbnt()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    addressFlag=false;
    profFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 13;
  }

  static std::string messageGetName()
  {
    return "UpdateProfileAbnt";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(addressFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="address=";
      rv+=address;
    }
    if(profFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="prof=";
      rv+='(';
      rv+=prof.toString();
      rv+=')';
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(addressFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(address);
    }
    if(profFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=prof.length<DataStream>();
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getAddress()const
  {
    if(!addressFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("address");
    }
    return address;
  }
  void setAddress(const std::string& argValue)
  {
    address=argValue;
    addressFlag=true;
  }
  std::string& getAddressRef()
  {
    addressFlag=true;
    return address;
  }
  bool hasAddress()const
  {
    return addressFlag;
  }
  const Profile& getProf()const
  {
    if(!profFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("prof");
    }
    return prof;
  }
  void setProf(const Profile& argValue)
  {
    prof=argValue;
    profFlag=true;
  }
  Profile& getProfRef()
  {
    profFlag=true;
    return prof;
  }
  bool hasProf()const
  {
    return profFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!addressFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("address");
    }
    if(!profFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("prof");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(addressTag);
    ds.writeStrLV(address); 
    ds.writeTag(profTag);
    ds.writeLength(prof.length<DataStream>());
    prof.serialize(ds);
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
    //  throw protogen::framework::IncompatibleVersionException("UpdateProfileAbnt");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case addressTag:
        {
          if(addressFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("address");
          }
          address=ds.readStrLV();
          addressFlag=true;
        }break;
        case profTag:
        {
          if(profFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("prof");
          }

          ds.readLength();prof.deserialize(ds);
          profFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("UpdateProfileAbnt",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!addressFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("address");
    }
    if(!profFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("prof");
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

  static const int32_t addressTag=1;
  static const int32_t profTag=2;

  int32_t seqNum;

  std::string address;
  Profile prof;

  bool addressFlag;
  bool profFlag;
};

}
}
}
}
}
#endif
