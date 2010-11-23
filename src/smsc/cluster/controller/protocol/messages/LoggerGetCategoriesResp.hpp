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
    categories.clear();
  }
 
  static int32_t messageGetTag()
  {
    return 1023;
  }

  static std::string messageGetName()
  {
    return "LoggerGetCategoriesResp";
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
      for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;++it)
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
    if(categoriesFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;++it)
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
  const std::vector<CategoryInfo>& getCategories()const
  {
    if(!categoriesFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("categories");
    }
    return categories;
  }
  void setCategories(const std::vector<CategoryInfo>& argValue)
  {
    categories=argValue;
    categoriesFlag=true;
  }
  std::vector<CategoryInfo>& getCategoriesRef()
  {
    categoriesFlag=true;
    return categories;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    if(categoriesFlag)
    {
      ds.writeTag(categoriesTag);
    typename DataStream::LengthType len=0;
    for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;++it)
    {
      len+=it->length<DataStream>();
    }
    ds.writeLength(len);
    for(std::vector<CategoryInfo>::const_iterator it=categories.begin(),end=categories.end();it!=end;++it)
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
    //  throw protogen::framework::IncompatibleVersionException("LoggerGetCategoriesResp");
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
        case categoriesTag:
        {
          if(categoriesFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("categories");
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
  static const int32_t categoriesTag=2;

  int32_t seqNum;

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
