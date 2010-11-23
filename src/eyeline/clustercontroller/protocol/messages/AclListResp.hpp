#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLLISTRESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_ACLLISTRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "AclInfo.hpp"


#ident "@(#) AclListResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class AclListResp{
public:
  AclListResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    aclListFlag=false;
    aclList.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 1026;
  }

  static std::string messageGetName()
  {
    return "AclListResp";
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
    if(aclListFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="aclList=";
      rv+="[";
      bool first=true;
      for(std::vector<AclInfo>::const_iterator it=aclList.begin(),end=aclList.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+="(";
        rv+=it->toString();
        rv+=")";
      }
      rv+="]";
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
    if(aclListFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<AclInfo>::const_iterator it=aclList.begin(),end=aclList.end();it!=end;++it)
      {
        rv+=it->length<DataStream>();
      }
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
  const std::vector<AclInfo>& getAclList()const
  {
    if(!aclListFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("aclList");
    }
    return aclList;
  }
  void setAclList(const std::vector<AclInfo>& argValue)
  {
    aclList=argValue;
    aclListFlag=true;
  }
  std::vector<AclInfo>& getAclListRef()
  {
    aclListFlag=true;
    return aclList;
  }
  bool hasAclList()const
  {
    return aclListFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    if(aclListFlag)
    {
      ds.writeTag(aclListTag);
    typename DataStream::LengthType len=0;
    for(std::vector<AclInfo>::const_iterator it=aclList.begin(),end=aclList.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<AclInfo>::const_iterator it=aclList.begin(),end=aclList.end();it!=end;++it)
    {
      it->serialize(ds);
    }
    }
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
    //  throw protogen::framework::IncompatibleVersionException("AclListResp");
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
        case aclListTag:
        {
          if(aclListFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("aclList");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            aclList.push_back(AclInfo());
            aclList.back().deserialize(ds);
            rd+=aclList.back().length<DataStream>();
          }
          aclListFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("AclListResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
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
  static const int32_t aclListTag=2;

  int32_t seqNum;

  Response resp;
  std::vector<AclInfo> aclList;

  bool respFlag;
  bool aclListFlag;
};

}
}
}
}
#endif
