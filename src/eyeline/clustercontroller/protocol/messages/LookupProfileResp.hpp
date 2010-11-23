#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_LOOKUPPROFILERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_LOOKUPPROFILERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "ProfileMatchType.hpp"
#include "Profile.hpp"


#ident "@(#) LookupProfileResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class LookupProfileResp{
public:
  LookupProfileResp()
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
 
  static int32_t messageGetTag()
  {
    return 1011;
  }

  static std::string messageGetName()
  {
    return "LookupProfileResp";
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
      rv+=ProfileMatchType::getNameByValue(matchType);
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
      rv+=DataStream::fieldSize(matchType.getValue());
 
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
      throw eyeline::protogen::framework::FieldIsNullException("resp");
    }
    return resp;
  }
  void setResp(const Response& argValue)
  {
    resp=argValue;
    respFlag=true;
  }
  Response& getRespRef()
  {
    respFlag=true;
    return resp;
  }
  bool hasResp()const
  {
    return respFlag;
  }
  const ProfileMatchType& getMatchType()const
  {
    if(!matchTypeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("matchType");
    }
    return matchType;
  }
  void setMatchType(const ProfileMatchType& argValue)
  {
    matchType=argValue;
    matchTypeFlag=true;
  }
  ProfileMatchType& getMatchTypeRef()
  {
    matchTypeFlag=true;
    return matchType;
  }
  bool hasMatchType()const
  {
    return matchTypeFlag;
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
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!matchTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("matchType");
    }
    if(!profFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("prof");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(matchTypeTag);
    ds.writeByteLV(matchType.getValue());
 
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
    //  throw protogen::framework::IncompatibleVersionException("LookupProfileResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case respTag:
        {
          if(respFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("resp");
          }

          ds.readLength();resp.deserialize(ds);
          respFlag=true;
        }break;
        case matchTypeTag:
        {
          if(matchTypeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("matchType");
          }
          matchType=ds.readByteLV();
          matchTypeFlag=true;
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
          //  throw protogen::framework::UnexpectedTag("LookupProfileResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!matchTypeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("matchType");
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

  static const int32_t respTag=1;
  static const int32_t matchTypeTag=2;
  static const int32_t profTag=3;

  int32_t seqNum;

  Response resp;
  ProfileMatchType matchType;
  Profile prof;

  bool respFlag;
  bool matchTypeFlag;
  bool profFlag;
};

}
}
}
}
#endif
