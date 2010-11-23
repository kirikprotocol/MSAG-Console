#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_MULTIRESPONSE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_MULTIRESPONSE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) MultiResponse version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class MultiResponse{
public:
  MultiResponse()
  {
    Clear();
  }
  void Clear()
  {
    statusFlag=false;
    status.clear();
    idsFlag=false;
    ids.clear();
  }
 

  static std::string messageGetName()
  {
    return "MultiResponse";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(statusFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="status=";
      rv+="[";
      bool first=true;
      for(std::vector<int32_t>::const_iterator it=status.begin(),end=status.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        sprintf(buf,"%d",*it);
        rv+=buf;
      }
      rv+="]";
    }
    if(idsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="ids=";
      rv+="[";
      bool first=true;
      for(std::vector<int8_t>::const_iterator it=ids.begin(),end=ids.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        sprintf(buf,"%d",(int)*it);
        rv+=buf;
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(statusFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(status);
    }
    if(idsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(ids);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<int32_t>& getStatus()const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("status");
    }
    return status;
  }
  void setStatus(const std::vector<int32_t>& argValue)
  {
    status=argValue;
    statusFlag=true;
  }
  std::vector<int32_t>& getStatusRef()
  {
    statusFlag=true;
    return status;
  }
  bool hasStatus()const
  {
    return statusFlag;
  }
  const std::vector<int8_t>& getIds()const
  {
    if(!idsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("ids");
    }
    return ids;
  }
  void setIds(const std::vector<int8_t>& argValue)
  {
    ids=argValue;
    idsFlag=true;
  }
  std::vector<int8_t>& getIdsRef()
  {
    idsFlag=true;
    return ids;
  }
  bool hasIds()const
  {
    return idsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!idsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("ids");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(statusTag);
    ds.writeLength(DataStream::fieldSize(status));
    for(std::vector<int32_t>::const_iterator it=status.begin(),end=status.end();it!=end;++it)
    {
      ds.writeInt32(*it);
          }
    ds.writeTag(idsTag);
    ds.writeLength(DataStream::fieldSize(ids));
    for(std::vector<int8_t>::const_iterator it=ids.begin(),end=ids.end();it!=end;++it)
    {
      ds.writeByte(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("MultiResponse");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case statusTag:
        {
          if(statusFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("status");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            status.push_back(ds.readInt32());
            rd+=DataStream::fieldSize(status.back());
          }
          statusFlag=true;
        }break;
        case idsTag:
        {
          if(idsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("ids");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            ids.push_back(ds.readByte());
            rd+=DataStream::fieldSize(ids.back());
          }
          idsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("MultiResponse",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!statusFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("status");
    }
    if(!idsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("ids");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t statusTag=1;
  static const int32_t idsTag=2;


  std::vector<int32_t> status;
  std::vector<int8_t> ids;

  bool statusFlag;
  bool idsFlag;
};

}
}
}
}
}
#endif
