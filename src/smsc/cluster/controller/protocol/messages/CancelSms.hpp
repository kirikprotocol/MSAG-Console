#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CANCELSMS_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_CANCELSMS_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CancelSms version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class CancelSms{
public:
  CancelSms()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    idsFlag=false;
    ids.clear();
    srcsFlag=false;
    srcs.clear();
    dstsFlag=false;
    dsts.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 15;
  }

  static std::string messageGetName()
  {
    return "CancelSms";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(idsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="ids=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=ids.begin(),end=ids.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    if(srcsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="srcs=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=srcs.begin(),end=srcs.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    if(dstsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="dsts=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=dsts.begin(),end=dsts.end();it!=end;++it)
      {
        if(first)
        {
          first=false;
        }else
        {
          rv+=",";
        }
        rv+=*it;
      }
      rv+="]";
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(idsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(ids);
    }
    if(srcsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(srcs);
    }
    if(dstsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dsts);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::vector<std::string>& getIds()const
  {
    if(!idsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("ids");
    }
    return ids;
  }
  void setIds(const std::vector<std::string>& argValue)
  {
    ids=argValue;
    idsFlag=true;
  }
  std::vector<std::string>& getIdsRef()
  {
    idsFlag=true;
    return ids;
  }
  bool hasIds()const
  {
    return idsFlag;
  }
  const std::vector<std::string>& getSrcs()const
  {
    if(!srcsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("srcs");
    }
    return srcs;
  }
  void setSrcs(const std::vector<std::string>& argValue)
  {
    srcs=argValue;
    srcsFlag=true;
  }
  std::vector<std::string>& getSrcsRef()
  {
    srcsFlag=true;
    return srcs;
  }
  bool hasSrcs()const
  {
    return srcsFlag;
  }
  const std::vector<std::string>& getDsts()const
  {
    if(!dstsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("dsts");
    }
    return dsts;
  }
  void setDsts(const std::vector<std::string>& argValue)
  {
    dsts=argValue;
    dstsFlag=true;
  }
  std::vector<std::string>& getDstsRef()
  {
    dstsFlag=true;
    return dsts;
  }
  bool hasDsts()const
  {
    return dstsFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!idsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("ids");
    }
    if(!srcsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("srcs");
    }
    if(!dstsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("dsts");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idsTag);
    ds.writeLength(DataStream::fieldSize(ids));
    for(std::vector<std::string>::const_iterator it=ids.begin(),end=ids.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    ds.writeTag(srcsTag);
    ds.writeLength(DataStream::fieldSize(srcs));
    for(std::vector<std::string>::const_iterator it=srcs.begin(),end=srcs.end();it!=end;++it)
    {
      ds.writeStr(*it);
          }
    ds.writeTag(dstsTag);
    ds.writeLength(DataStream::fieldSize(dsts));
    for(std::vector<std::string>::const_iterator it=dsts.begin(),end=dsts.end();it!=end;++it)
    {
      ds.writeStr(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("CancelSms");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case idsTag:
        {
          if(idsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("ids");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            ids.push_back(ds.readStr());
            rd+=DataStream::fieldSize(ids.back());
            rd+=DataStream::lengthTypeSize;
          }
          idsFlag=true;
        }break;
        case srcsTag:
        {
          if(srcsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("srcs");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            srcs.push_back(ds.readStr());
            rd+=DataStream::fieldSize(srcs.back());
            rd+=DataStream::lengthTypeSize;
          }
          srcsFlag=true;
        }break;
        case dstsTag:
        {
          if(dstsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("dsts");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            dsts.push_back(ds.readStr());
            rd+=DataStream::fieldSize(dsts.back());
            rd+=DataStream::lengthTypeSize;
          }
          dstsFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CancelSms",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("ids");
    }
    if(!srcsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("srcs");
    }
    if(!dstsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("dsts");
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

  static const int32_t idsTag=1;
  static const int32_t srcsTag=2;
  static const int32_t dstsTag=3;

  int32_t seqNum;

  std::vector<std::string> ids;
  std::vector<std::string> srcs;
  std::vector<std::string> dsts;

  bool idsFlag;
  bool srcsFlag;
  bool dstsFlag;
};

}
}
}
}
}
#endif
