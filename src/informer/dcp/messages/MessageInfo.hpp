#ifndef __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_MESSAGEINFO_HPP__
#define __GENERATED_MESSAGE_EYELINE_INFORMER_DCP_MESSAGES_MESSAGEINFO_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "DeliveryMessageState.hpp"


#ident "@(#) MessageInfo version 1.0"



namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class MessageInfo{
public:
  MessageInfo()
  {
    Clear();
  }
  void Clear()
  {
    idFlag=false;
    stateFlag=false;
    dateFlag=false;
    abonentFlag=false;
    textFlag=false;
    errorCodeFlag=false;
    userDataFlag=false;
  }
 

  static std::string messageGetName()
  {
    return "MessageInfo";
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(idFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="id=";
      sprintf(buf,"%lld",id);
      rv+=buf;
    }
    if(stateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="state=";
      rv+=DeliveryMessageState::getNameByValue(state);
    }
    if(dateFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="date=";
      rv+=date;
    }
    if(abonentFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="abonent=";
      rv+=abonent;
    }
    if(textFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="text=";
      rv+=text;
    }
    if(errorCodeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="errorCode=";
      sprintf(buf,"%d",errorCode);
      rv+=buf;
    }
    if(userDataFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="userData=";
      rv+=userData;
    }
    return rv;
  }

  template <class DataStream>
  int32_t length()const
  {
    int32_t rv=0;
    if(idFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(id);
    }
    if(stateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(state.getValue());
 
    }
    if(dateFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(date);
    }
    if(abonentFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(abonent);
    }
    if(textFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(text);
    }
    if(errorCodeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(errorCode);
    }
    if(userDataFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(userData);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  int64_t getId()const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("id");
    }
    return id;
  }
  void setId(int64_t argValue)
  {
    id=argValue;
    idFlag=true;
  }
  int64_t& getIdRef()
  {
    idFlag=true;
    return id;
  }
  bool hasId()const
  {
    return idFlag;
  }
  const DeliveryMessageState& getState()const
  {
    if(!stateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("state");
    }
    return state;
  }
  void setState(const DeliveryMessageState& argValue)
  {
    state=argValue;
    stateFlag=true;
  }
  DeliveryMessageState& getStateRef()
  {
    stateFlag=true;
    return state;
  }
  bool hasState()const
  {
    return stateFlag;
  }
  const std::string& getDate()const
  {
    if(!dateFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("date");
    }
    return date;
  }
  void setDate(const std::string& argValue)
  {
    date=argValue;
    dateFlag=true;
  }
  std::string& getDateRef()
  {
    dateFlag=true;
    return date;
  }
  bool hasDate()const
  {
    return dateFlag;
  }
  const std::string& getAbonent()const
  {
    if(!abonentFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("abonent");
    }
    return abonent;
  }
  void setAbonent(const std::string& argValue)
  {
    abonent=argValue;
    abonentFlag=true;
  }
  std::string& getAbonentRef()
  {
    abonentFlag=true;
    return abonent;
  }
  bool hasAbonent()const
  {
    return abonentFlag;
  }
  const std::string& getText()const
  {
    if(!textFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("text");
    }
    return text;
  }
  void setText(const std::string& argValue)
  {
    text=argValue;
    textFlag=true;
  }
  std::string& getTextRef()
  {
    textFlag=true;
    return text;
  }
  bool hasText()const
  {
    return textFlag;
  }
  int32_t getErrorCode()const
  {
    if(!errorCodeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("errorCode");
    }
    return errorCode;
  }
  void setErrorCode(int32_t argValue)
  {
    errorCode=argValue;
    errorCodeFlag=true;
  }
  int32_t& getErrorCodeRef()
  {
    errorCodeFlag=true;
    return errorCode;
  }
  bool hasErrorCode()const
  {
    return errorCodeFlag;
  }
  const std::string& getUserData()const
  {
    if(!userDataFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("userData");
    }
    return userData;
  }
  void setUserData(const std::string& argValue)
  {
    userData=argValue;
    userDataFlag=true;
  }
  std::string& getUserDataRef()
  {
    userDataFlag=true;
    return userData;
  }
  bool hasUserData()const
  {
    return userDataFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(idTag);
    ds.writeInt64LV(id); 
    if(stateFlag)
    {
      ds.writeTag(stateTag);
    ds.writeByteLV(state.getValue());
 
    }
    if(dateFlag)
    {
      ds.writeTag(dateTag);
    ds.writeStrLV(date); 
    }
    if(abonentFlag)
    {
      ds.writeTag(abonentTag);
    ds.writeStrLV(abonent); 
    }
    if(textFlag)
    {
      ds.writeTag(textTag);
    ds.writeStrLV(text); 
    }
    if(errorCodeFlag)
    {
      ds.writeTag(errorCodeTag);
    ds.writeInt32LV(errorCode); 
    }
    if(userDataFlag)
    {
      ds.writeTag(userDataTag);
    ds.writeStrLV(userData); 
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
    //  throw protogen::framework::IncompatibleVersionException("MessageInfo");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      typename DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case idTag:
        {
          if(idFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("id");
          }
          id=ds.readInt64LV();
          idFlag=true;
        }break;
        case stateTag:
        {
          if(stateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("state");
          }
          state=ds.readByteLV();
          stateFlag=true;
        }break;
        case dateTag:
        {
          if(dateFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("date");
          }
          date=ds.readStrLV();
          dateFlag=true;
        }break;
        case abonentTag:
        {
          if(abonentFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("abonent");
          }
          abonent=ds.readStrLV();
          abonentFlag=true;
        }break;
        case textTag:
        {
          if(textFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("text");
          }
          text=ds.readStrLV();
          textFlag=true;
        }break;
        case errorCodeTag:
        {
          if(errorCodeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("errorCode");
          }
          errorCode=ds.readInt32LV();
          errorCodeFlag=true;
        }break;
        case userDataTag:
        {
          if(userDataFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("userData");
          }
          userData=ds.readStrLV();
          userDataFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("MessageInfo",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!idFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("id");
    }

  }


  void messageSetConnId(int argConnId)
  {
    connId=argConnId;
  }

  int messageGetConnId()const
  {
    return connId;
  }
 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t idTag=1;
  static const int32_t stateTag=2;
  static const int32_t dateTag=3;
  static const int32_t abonentTag=4;
  static const int32_t textTag=5;
  static const int32_t errorCodeTag=6;
  static const int32_t userDataTag=7;

  int connId;

  int64_t id;
  DeliveryMessageState state;
  std::string date;
  std::string abonent;
  std::string text;
  int32_t errorCode;
  std::string userData;

  bool idFlag;
  bool stateFlag;
  bool dateFlag;
  bool abonentFlag;
  bool textFlag;
  bool errorCodeFlag;
  bool userDataFlag;
};

}
}
}
}
#endif
