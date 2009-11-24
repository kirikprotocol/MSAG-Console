#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLLIST_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_DLLIST_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) DlList version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class DlList{
public:
  DlList()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    namesFlag=false;
    ownersFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(namesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="names=";
      rv+="[";
      bool first=true;
      for(string_list::const_iterator it=names.begin(),end=names.end();it!=end;it++)
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
    if(ownersFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="owners=";
      rv+="[";
      bool first=true;
      for(string_list::const_iterator it=owners.begin(),end=owners.end();it!=end;it++)
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
    if(namesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(names);
    }
    if(ownersFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(owners);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const string_list& getNames()const
  {
    if(!namesFlag)
    {
      throw protogen::framework::FieldIsNullException("names");
    }
    return names;
  }
  void setNames(const string_list& value)
  {
    names=value;
    namesFlag=true;
  }
  bool hasNames()const
  {
    return namesFlag;
  }
  const string_list& getOwners()const
  {
    if(!ownersFlag)
    {
      throw protogen::framework::FieldIsNullException("owners");
    }
    return owners;
  }
  void setOwners(const string_list& value)
  {
    owners=value;
    ownersFlag=true;
  }
  bool hasOwners()const
  {
    return ownersFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!namesFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("names");
    }
    if(!ownersFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("owners");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(namesTag);
    ds.writeStrLstLV(names);
    ds.writeTag(ownersTag);
    ds.writeStrLstLV(owners);
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
    //  throw protogen::framework::IncompatibleVersionException("DlList");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case namesTag:
        {
          if(namesFlag)
          {
            throw protogen::framework::DuplicateFieldException("names");
          }
          ds.readStrLstLV(names);
          namesFlag=true;
        }break;
        case ownersTag:
        {
          if(ownersFlag)
          {
            throw protogen::framework::DuplicateFieldException("owners");
          }
          ds.readStrLstLV(owners);
          ownersFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("DlList",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!namesFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("names");
    }
    if(!ownersFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("owners");
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

  static const uint32_t namesTag=1;
  static const uint32_t ownersTag=2;

  uint32_t seqNum;

  string_list names;
  string_list owners;

  bool namesFlag;
  bool ownersFlag;
};

}
}
}
}
}
#endif
