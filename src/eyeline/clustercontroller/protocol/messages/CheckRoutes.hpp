#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_CHECKROUTES_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_CHECKROUTES_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) CheckRoutes version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class CheckRoutes{
public:
  CheckRoutes()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    fileNameFlag=false;
  }
 
  static int32_t messageGetTag()
  {
    return 9;
  }

  static std::string messageGetName()
  {
    return "CheckRoutes";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    sprintf(buf,"seqNum=%d",seqNum);
    rv+=buf;
    if(fileNameFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="fileName=";
      rv+=fileName;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(fileNameFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(fileName);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getFileName()const
  {
    if(!fileNameFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("fileName");
    }
    return fileName;
  }
  void setFileName(const std::string& argValue)
  {
    fileName=argValue;
    fileNameFlag=true;
  }
  std::string& getFileNameRef()
  {
    fileNameFlag=true;
    return fileName;
  }
  bool hasFileName()const
  {
    return fileNameFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!fileNameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("fileName");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(fileNameTag);
    ds.writeStrLV(fileName); 
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
    //  throw protogen::framework::IncompatibleVersionException("CheckRoutes");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case fileNameTag:
        {
          if(fileNameFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("fileName");
          }
          fileName=ds.readStrLV();
          fileNameFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("CheckRoutes",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!fileNameFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("fileName");
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

  static const int32_t fileNameTag=1;

  int32_t seqNum;

  std::string fileName;

  bool fileNameFlag;
};

}
}
}
}
#endif
