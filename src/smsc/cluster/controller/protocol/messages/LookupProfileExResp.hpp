#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_LOOKUPPROFILEEXRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_LOOKUPPROFILEEXRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "Profile.hpp"


#ident "@(#) LookupProfileExResp version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class LookupProfileExResp{
public:
  LookupProfileExResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    matchTypeFlag=false;
    profFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(respFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="resp=";
      rv+='(';
      rv+=resp.toString();
      rv+=')';
    }
    if(matchTypeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="matchType=";
      sprintf(buf,"%u",(unsigned int)matchType);
      rv+=buf;
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
  uint32_t length()const
  {
    uint32_t rv=0;
    if(respFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=resp.length<DataStream>();
    }
    if(matchTypeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(matchType);
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
  const Response& getResp()const
  {
    if(!respFlag)
    {
      throw protogen::framework::FieldIsNullException("resp");
    }
    return resp;
  }
  void setResp(const Response& value)
  {
    resp=value;
    respFlag=true;
  }
  bool hasResp()const
  {
    return respFlag;
  }
  uint8_t getMatchType()const
  {
    if(!matchTypeFlag)
    {
      throw protogen::framework::FieldIsNullException("matchType");
    }
    return matchType;
  }
  void setMatchType(uint8_t value)
  {
    matchType=value;
    matchTypeFlag=true;
  }
  bool hasMatchType()const
  {
    return matchTypeFlag;
  }
  const Profile& getProf()const
  {
    if(!profFlag)
    {
      throw protogen::framework::FieldIsNullException("prof");
    }
    return prof;
  }
  void setProf(const Profile& value)
  {
    prof=value;
    profFlag=true;
  }
  bool hasProf()const
  {
    return profFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!matchTypeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("matchType");
    }
    if(!profFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("prof");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(matchTypeTag);
    ds.writeByteLV(matchType);
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
    //uint8_t rdVersionMajor=ds.readByte();
    //uint8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("LookupProfileExResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case respTag:
        {
          if(respFlag)
          {
            throw protogen::framework::DuplicateFieldException("resp");
          }

          ds.readLength();resp.deserialize(ds);
          respFlag=true;
        }break;
        case matchTypeTag:
        {
          if(matchTypeFlag)
          {
            throw protogen::framework::DuplicateFieldException("matchType");
          }
          matchType=ds.readByteLV();
          matchTypeFlag=true;
        }break;
        case profTag:
        {
          if(profFlag)
          {
            throw protogen::framework::DuplicateFieldException("prof");
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
          //  throw protogen::framework::UnexpectedTag("LookupProfileExResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!matchTypeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("matchType");
    }
    if(!profFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("prof");
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

  static const uint32_t respTag=1;
  static const uint32_t matchTypeTag=2;
  static const uint32_t profTag=3;

  uint32_t seqNum;

  Response resp;
  uint8_t matchType;
  Profile prof;

  bool respFlag;
  bool matchTypeFlag;
  bool profFlag;
};

}
}
}
}
}
#endif
