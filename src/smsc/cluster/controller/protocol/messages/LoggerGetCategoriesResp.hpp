#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_LOGGERGETCATEGORIESRESP_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_LOGGERGETCATEGORIESRESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "CategoryInfo.hpp"


#ident "@(#) LoggerGetCategoriesResp version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class LoggerGetCategoriesResp{
public:
  LoggerGetCategoriesResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    categoriesFlag=false;
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
    if(categoriesFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="categories=";
      rv+="[";
      bool first=true;
      for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;it++)
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
  uint32_t length()const
  {
    uint32_t rv=0;
    if(respFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=resp.length<DataStream>();
    }
    if(categoriesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;it++)
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
  const std::vector<CategoryInfo>& getCategories()const
  {
    if(!categoriesFlag)
    {
      throw protogen::framework::FieldIsNullException("categories");
    }
    return categories;
  }
  void setCategories(const std::vector<CategoryInfo>& value)
  {
    categories=value;
    categoriesFlag=true;
  }
  bool hasCategories()const
  {
    return categoriesFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!categoriesFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("categories");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(categoriesTag);
    typename DataStream::LengthType len=0;
    for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;it++)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;it++)
    {
      it->serialize(ds);
    }
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
    //  throw protogen::framework::IncompatibleVersionException("LoggerGetCategoriesResp");
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
        case categoriesTag:
        {
          if(categoriesFlag)
          {
            throw protogen::framework::DuplicateFieldException("categories");
          }

          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            categories.push_back(CategoryInfo());
            categories.back().deserialize(ds);
            rd+=categories.back().length<DataStream>();
          }
          categoriesFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("LoggerGetCategoriesResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!categoriesFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("categories");
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
  static const uint32_t categoriesTag=2;

  uint32_t seqNum;

  Response resp;
  std::vector<CategoryInfo> categories;

  bool respFlag;
  bool categoriesFlag;
};

}
}
}
}
}
#endif
