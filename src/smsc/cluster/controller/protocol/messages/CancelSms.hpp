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

typedef std::vector<std::string> string_list;

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
    srcsFlag=false;
    dstsFlag=false;
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
      for(string_list::const_iterator it=ids.begin(),end=ids.end();it!=end;it++)
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
      for(string_list::const_iterator it=srcs.begin(),end=srcs.end();it!=end;it++)
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
      for(string_list::const_iterator it=dsts.begin(),end=dsts.end();it!=end;it++)
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
  uint32_t length()const
  {
    uint32_t rv=0;
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
  const string_list& getIds()const
  {
    if(!idsFlag)
    {
      throw protogen::framework::FieldIsNullException("ids");
    }
    return ids;
  }
  void setIds(const string_list& value)
  {
    ids=value;
    idsFlag=true;
  }
  bool hasIds()const
  {
    return idsFlag;
  }
  const string_list& getSrcs()const
  {
    if(!srcsFlag)
    {
      throw protogen::framework::FieldIsNullException("srcs");
    }
    return srcs;
  }
  void setSrcs(const string_list& value)
  {
    srcs=value;
    srcsFlag=true;
  }
  bool hasSrcs()const
  {
    return srcsFlag;
  }
  const string_list& getDsts()const
  {
    if(!dstsFlag)
    {
      throw protogen::framework::FieldIsNullException("dsts");
    }
    return dsts;
  }
  void setDsts(const string_list& value)
  {
    dsts=value;
    dstsFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("ids");
    }
    if(!srcsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("srcs");
    }
    if(!dstsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dsts");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idsTag);
    ds.writeStrLstLV(ids);
    ds.writeTag(srcsTag);
    ds.writeStrLstLV(srcs);
    ds.writeTag(dstsTag);
    ds.writeStrLstLV(dsts);
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
    //  throw protogen::framework::IncompatibleVersionException("CancelSms");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case idsTag:
        {
          if(idsFlag)
          {
            throw protogen::framework::DuplicateFieldException("ids");
          }
          ds.readStrLstLV(ids);
          idsFlag=true;
        }break;
        case srcsTag:
        {
          if(srcsFlag)
          {
            throw protogen::framework::DuplicateFieldException("srcs");
          }
          ds.readStrLstLV(srcs);
          srcsFlag=true;
        }break;
        case dstsTag:
        {
          if(dstsFlag)
          {
            throw protogen::framework::DuplicateFieldException("dsts");
          }
          ds.readStrLstLV(dsts);
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
      throw protogen::framework::MandatoryFieldMissingException("ids");
    }
    if(!srcsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("srcs");
    }
    if(!dstsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("dsts");
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

  static const uint32_t idsTag=1;
  static const uint32_t srcsTag=2;
  static const uint32_t dstsTag=3;

  uint32_t seqNum;

  string_list ids;
  string_list srcs;
  string_list dsts;

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
