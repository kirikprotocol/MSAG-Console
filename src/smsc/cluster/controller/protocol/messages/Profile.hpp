#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_PROFILE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_PROFILE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"


#ident "@(#) Profile version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

class Profile{
public:
  Profile()
  {
    Clear();
  }
  void Clear()
  {
    divertFlag=false;
    localeFlag=false;
    codepageFlag=false;
    reportOptionsFlag=false;
    hideFlag=false;
    hideModifiableFlag=false;
    divertActiveFlag=false;
    divertActiveAbsentFlag=false;
    divertActiveBlockedFlag=false;
    divertActiveBarredFlag=false;
    divertActiveCapacityFlag=false;
    divertModifiableFlag=false;
    udhConcatFlag=false;
    translitFlag=false;
    closedGroupIdFlag=false;
    accessMaskInFlag=false;
    accessMaskOutFlag=false;
    subscriptionFlag=false;
    sponsoredFlag=false;
    nickFlag=false;
  }

  std::string toString()const
  {
    std::string rv;
    char buf[32];
    if(divertFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divert=";
      rv+=divert;
    }
    if(localeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="locale=";
      rv+=locale;
    }
    if(codepageFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="codepage=";
      sprintf(buf,"%u",(unsigned int)codepage);
      rv+=buf;
    }
    if(reportOptionsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="reportOptions=";
      sprintf(buf,"%u",(unsigned int)reportOptions);
      rv+=buf;
    }
    if(hideFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="hide=";
      sprintf(buf,"%u",(unsigned int)hide);
      rv+=buf;
    }
    if(hideModifiableFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="hideModifiable=";
      rv+=hideModifiable?"true":"false";
    }
    if(divertActiveFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divertActive=";
      rv+=divertActive?"true":"false";
    }
    if(divertActiveAbsentFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divertActiveAbsent=";
      rv+=divertActiveAbsent?"true":"false";
    }
    if(divertActiveBlockedFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divertActiveBlocked=";
      rv+=divertActiveBlocked?"true":"false";
    }
    if(divertActiveBarredFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divertActiveBarred=";
      rv+=divertActiveBarred?"true":"false";
    }
    if(divertActiveCapacityFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divertActiveCapacity=";
      rv+=divertActiveCapacity?"true":"false";
    }
    if(divertModifiableFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="divertModifiable=";
      rv+=divertModifiable?"true":"false";
    }
    if(udhConcatFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="udhConcat=";
      rv+=udhConcat?"true":"false";
    }
    if(translitFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="translit=";
      rv+=translit?"true":"false";
    }
    if(closedGroupIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="closedGroupId=";
      sprintf(buf,"%u",(unsigned int)closedGroupId);
      rv+=buf;
    }
    if(accessMaskInFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="accessMaskIn=";
      sprintf(buf,"%u",(unsigned int)accessMaskIn);
      rv+=buf;
    }
    if(accessMaskOutFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="accessMaskOut=";
      sprintf(buf,"%u",(unsigned int)accessMaskOut);
      rv+=buf;
    }
    if(subscriptionFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="subscription=";
      sprintf(buf,"%u",(unsigned int)subscription);
      rv+=buf;
    }
    if(sponsoredFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="sponsored=";
      sprintf(buf,"%u",(unsigned int)sponsored);
      rv+=buf;
    }
    if(nickFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="nick=";
      rv+=nick;
    }
    return rv;
  }

  template <class DataStream>
  uint32_t length()const
  {
    uint32_t rv=0;
    if(divertFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divert);
    }
    if(localeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(locale);
    }
    if(codepageFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(codepage);
    }
    if(reportOptionsFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(reportOptions);
    }
    if(hideFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(hide);
    }
    if(hideModifiableFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(hideModifiable);
    }
    if(divertActiveFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divertActive);
    }
    if(divertActiveAbsentFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divertActiveAbsent);
    }
    if(divertActiveBlockedFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divertActiveBlocked);
    }
    if(divertActiveBarredFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divertActiveBarred);
    }
    if(divertActiveCapacityFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divertActiveCapacity);
    }
    if(divertModifiableFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(divertModifiable);
    }
    if(udhConcatFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(udhConcat);
    }
    if(translitFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(translit);
    }
    if(closedGroupIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(closedGroupId);
    }
    if(accessMaskInFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(accessMaskIn);
    }
    if(accessMaskOutFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(accessMaskOut);
    }
    if(subscriptionFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(subscription);
    }
    if(sponsoredFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(sponsored);
    }
    if(nickFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(nick);
    }
    rv+=DataStream::tagTypeSize;
    return rv;
  }
  const std::string& getDivert()const
  {
    if(!divertFlag)
    {
      throw protogen::framework::FieldIsNullException("divert");
    }
    return divert;
  }
  void setDivert(const std::string& value)
  {
    divert=value;
    divertFlag=true;
  }
  bool hasDivert()const
  {
    return divertFlag;
  }
  const std::string& getLocale()const
  {
    if(!localeFlag)
    {
      throw protogen::framework::FieldIsNullException("locale");
    }
    return locale;
  }
  void setLocale(const std::string& value)
  {
    locale=value;
    localeFlag=true;
  }
  bool hasLocale()const
  {
    return localeFlag;
  }
  uint8_t getCodepage()const
  {
    if(!codepageFlag)
    {
      throw protogen::framework::FieldIsNullException("codepage");
    }
    return codepage;
  }
  void setCodepage(uint8_t value)
  {
    codepage=value;
    codepageFlag=true;
  }
  bool hasCodepage()const
  {
    return codepageFlag;
  }
  uint8_t getReportOptions()const
  {
    if(!reportOptionsFlag)
    {
      throw protogen::framework::FieldIsNullException("reportOptions");
    }
    return reportOptions;
  }
  void setReportOptions(uint8_t value)
  {
    reportOptions=value;
    reportOptionsFlag=true;
  }
  bool hasReportOptions()const
  {
    return reportOptionsFlag;
  }
  uint8_t getHide()const
  {
    if(!hideFlag)
    {
      throw protogen::framework::FieldIsNullException("hide");
    }
    return hide;
  }
  void setHide(uint8_t value)
  {
    hide=value;
    hideFlag=true;
  }
  bool hasHide()const
  {
    return hideFlag;
  }
  bool getHideModifiable()const
  {
    if(!hideModifiableFlag)
    {
      throw protogen::framework::FieldIsNullException("hideModifiable");
    }
    return hideModifiable;
  }
  void setHideModifiable(bool value)
  {
    hideModifiable=value;
    hideModifiableFlag=true;
  }
  bool hasHideModifiable()const
  {
    return hideModifiableFlag;
  }
  bool getDivertActive()const
  {
    if(!divertActiveFlag)
    {
      throw protogen::framework::FieldIsNullException("divertActive");
    }
    return divertActive;
  }
  void setDivertActive(bool value)
  {
    divertActive=value;
    divertActiveFlag=true;
  }
  bool hasDivertActive()const
  {
    return divertActiveFlag;
  }
  bool getDivertActiveAbsent()const
  {
    if(!divertActiveAbsentFlag)
    {
      throw protogen::framework::FieldIsNullException("divertActiveAbsent");
    }
    return divertActiveAbsent;
  }
  void setDivertActiveAbsent(bool value)
  {
    divertActiveAbsent=value;
    divertActiveAbsentFlag=true;
  }
  bool hasDivertActiveAbsent()const
  {
    return divertActiveAbsentFlag;
  }
  bool getDivertActiveBlocked()const
  {
    if(!divertActiveBlockedFlag)
    {
      throw protogen::framework::FieldIsNullException("divertActiveBlocked");
    }
    return divertActiveBlocked;
  }
  void setDivertActiveBlocked(bool value)
  {
    divertActiveBlocked=value;
    divertActiveBlockedFlag=true;
  }
  bool hasDivertActiveBlocked()const
  {
    return divertActiveBlockedFlag;
  }
  bool getDivertActiveBarred()const
  {
    if(!divertActiveBarredFlag)
    {
      throw protogen::framework::FieldIsNullException("divertActiveBarred");
    }
    return divertActiveBarred;
  }
  void setDivertActiveBarred(bool value)
  {
    divertActiveBarred=value;
    divertActiveBarredFlag=true;
  }
  bool hasDivertActiveBarred()const
  {
    return divertActiveBarredFlag;
  }
  bool getDivertActiveCapacity()const
  {
    if(!divertActiveCapacityFlag)
    {
      throw protogen::framework::FieldIsNullException("divertActiveCapacity");
    }
    return divertActiveCapacity;
  }
  void setDivertActiveCapacity(bool value)
  {
    divertActiveCapacity=value;
    divertActiveCapacityFlag=true;
  }
  bool hasDivertActiveCapacity()const
  {
    return divertActiveCapacityFlag;
  }
  bool getDivertModifiable()const
  {
    if(!divertModifiableFlag)
    {
      throw protogen::framework::FieldIsNullException("divertModifiable");
    }
    return divertModifiable;
  }
  void setDivertModifiable(bool value)
  {
    divertModifiable=value;
    divertModifiableFlag=true;
  }
  bool hasDivertModifiable()const
  {
    return divertModifiableFlag;
  }
  bool getUdhConcat()const
  {
    if(!udhConcatFlag)
    {
      throw protogen::framework::FieldIsNullException("udhConcat");
    }
    return udhConcat;
  }
  void setUdhConcat(bool value)
  {
    udhConcat=value;
    udhConcatFlag=true;
  }
  bool hasUdhConcat()const
  {
    return udhConcatFlag;
  }
  bool getTranslit()const
  {
    if(!translitFlag)
    {
      throw protogen::framework::FieldIsNullException("translit");
    }
    return translit;
  }
  void setTranslit(bool value)
  {
    translit=value;
    translitFlag=true;
  }
  bool hasTranslit()const
  {
    return translitFlag;
  }
  uint32_t getClosedGroupId()const
  {
    if(!closedGroupIdFlag)
    {
      throw protogen::framework::FieldIsNullException("closedGroupId");
    }
    return closedGroupId;
  }
  void setClosedGroupId(uint32_t value)
  {
    closedGroupId=value;
    closedGroupIdFlag=true;
  }
  bool hasClosedGroupId()const
  {
    return closedGroupIdFlag;
  }
  uint32_t getAccessMaskIn()const
  {
    if(!accessMaskInFlag)
    {
      throw protogen::framework::FieldIsNullException("accessMaskIn");
    }
    return accessMaskIn;
  }
  void setAccessMaskIn(uint32_t value)
  {
    accessMaskIn=value;
    accessMaskInFlag=true;
  }
  bool hasAccessMaskIn()const
  {
    return accessMaskInFlag;
  }
  uint32_t getAccessMaskOut()const
  {
    if(!accessMaskOutFlag)
    {
      throw protogen::framework::FieldIsNullException("accessMaskOut");
    }
    return accessMaskOut;
  }
  void setAccessMaskOut(uint32_t value)
  {
    accessMaskOut=value;
    accessMaskOutFlag=true;
  }
  bool hasAccessMaskOut()const
  {
    return accessMaskOutFlag;
  }
  uint32_t getSubscription()const
  {
    if(!subscriptionFlag)
    {
      throw protogen::framework::FieldIsNullException("subscription");
    }
    return subscription;
  }
  void setSubscription(uint32_t value)
  {
    subscription=value;
    subscriptionFlag=true;
  }
  bool hasSubscription()const
  {
    return subscriptionFlag;
  }
  uint8_t getSponsored()const
  {
    if(!sponsoredFlag)
    {
      throw protogen::framework::FieldIsNullException("sponsored");
    }
    return sponsored;
  }
  void setSponsored(uint8_t value)
  {
    sponsored=value;
    sponsoredFlag=true;
  }
  bool hasSponsored()const
  {
    return sponsoredFlag;
  }
  const std::string& getNick()const
  {
    if(!nickFlag)
    {
      throw protogen::framework::FieldIsNullException("nick");
    }
    return nick;
  }
  void setNick(const std::string& value)
  {
    nick=value;
    nickFlag=true;
  }
  bool hasNick()const
  {
    return nickFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!divertFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divert");
    }
    if(!localeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("locale");
    }
    if(!codepageFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("codepage");
    }
    if(!reportOptionsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("reportOptions");
    }
    if(!hideFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("hide");
    }
    if(!hideModifiableFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("hideModifiable");
    }
    if(!divertActiveFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActive");
    }
    if(!divertActiveAbsentFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveAbsent");
    }
    if(!divertActiveBlockedFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveBlocked");
    }
    if(!divertActiveBarredFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveBarred");
    }
    if(!divertActiveCapacityFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveCapacity");
    }
    if(!divertModifiableFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertModifiable");
    }
    if(!udhConcatFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("udhConcat");
    }
    if(!translitFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("translit");
    }
    if(!closedGroupIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("closedGroupId");
    }
    if(!accessMaskInFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("accessMaskIn");
    }
    if(!accessMaskOutFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("accessMaskOut");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(divertTag);
    ds.writeStrLV(divert);
    ds.writeTag(localeTag);
    ds.writeStrLV(locale);
    ds.writeTag(codepageTag);
    ds.writeByteLV(codepage);
    ds.writeTag(reportOptionsTag);
    ds.writeByteLV(reportOptions);
    ds.writeTag(hideTag);
    ds.writeByteLV(hide);
    ds.writeTag(hideModifiableTag);
    ds.writeBoolLV(hideModifiable);
    ds.writeTag(divertActiveTag);
    ds.writeBoolLV(divertActive);
    ds.writeTag(divertActiveAbsentTag);
    ds.writeBoolLV(divertActiveAbsent);
    ds.writeTag(divertActiveBlockedTag);
    ds.writeBoolLV(divertActiveBlocked);
    ds.writeTag(divertActiveBarredTag);
    ds.writeBoolLV(divertActiveBarred);
    ds.writeTag(divertActiveCapacityTag);
    ds.writeBoolLV(divertActiveCapacity);
    ds.writeTag(divertModifiableTag);
    ds.writeBoolLV(divertModifiable);
    ds.writeTag(udhConcatTag);
    ds.writeBoolLV(udhConcat);
    ds.writeTag(translitTag);
    ds.writeBoolLV(translit);
    ds.writeTag(closedGroupIdTag);
    ds.writeInt32LV(closedGroupId);
    ds.writeTag(accessMaskInTag);
    ds.writeInt32LV(accessMaskIn);
    ds.writeTag(accessMaskOutTag);
    ds.writeInt32LV(accessMaskOut);
    if(subscriptionFlag)
    {
      ds.writeTag(subscriptionTag);
    ds.writeInt32LV(subscription);
    }
    if(sponsoredFlag)
    {
      ds.writeTag(sponsoredTag);
    ds.writeByteLV(sponsored);
    }
    if(nickFlag)
    {
      ds.writeTag(nickTag);
    ds.writeStrLV(nick);
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
    //  throw protogen::framework::IncompatibleVersionException("Profile");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      uint32_t tag=ds.readTag();
      switch(tag)
      {
        case divertTag:
        {
          if(divertFlag)
          {
            throw protogen::framework::DuplicateFieldException("divert");
          }
          divert=ds.readStrLV();
          divertFlag=true;
        }break;
        case localeTag:
        {
          if(localeFlag)
          {
            throw protogen::framework::DuplicateFieldException("locale");
          }
          locale=ds.readStrLV();
          localeFlag=true;
        }break;
        case codepageTag:
        {
          if(codepageFlag)
          {
            throw protogen::framework::DuplicateFieldException("codepage");
          }
          codepage=ds.readByteLV();
          codepageFlag=true;
        }break;
        case reportOptionsTag:
        {
          if(reportOptionsFlag)
          {
            throw protogen::framework::DuplicateFieldException("reportOptions");
          }
          reportOptions=ds.readByteLV();
          reportOptionsFlag=true;
        }break;
        case hideTag:
        {
          if(hideFlag)
          {
            throw protogen::framework::DuplicateFieldException("hide");
          }
          hide=ds.readByteLV();
          hideFlag=true;
        }break;
        case hideModifiableTag:
        {
          if(hideModifiableFlag)
          {
            throw protogen::framework::DuplicateFieldException("hideModifiable");
          }
          hideModifiable=ds.readBoolLV();
          hideModifiableFlag=true;
        }break;
        case divertActiveTag:
        {
          if(divertActiveFlag)
          {
            throw protogen::framework::DuplicateFieldException("divertActive");
          }
          divertActive=ds.readBoolLV();
          divertActiveFlag=true;
        }break;
        case divertActiveAbsentTag:
        {
          if(divertActiveAbsentFlag)
          {
            throw protogen::framework::DuplicateFieldException("divertActiveAbsent");
          }
          divertActiveAbsent=ds.readBoolLV();
          divertActiveAbsentFlag=true;
        }break;
        case divertActiveBlockedTag:
        {
          if(divertActiveBlockedFlag)
          {
            throw protogen::framework::DuplicateFieldException("divertActiveBlocked");
          }
          divertActiveBlocked=ds.readBoolLV();
          divertActiveBlockedFlag=true;
        }break;
        case divertActiveBarredTag:
        {
          if(divertActiveBarredFlag)
          {
            throw protogen::framework::DuplicateFieldException("divertActiveBarred");
          }
          divertActiveBarred=ds.readBoolLV();
          divertActiveBarredFlag=true;
        }break;
        case divertActiveCapacityTag:
        {
          if(divertActiveCapacityFlag)
          {
            throw protogen::framework::DuplicateFieldException("divertActiveCapacity");
          }
          divertActiveCapacity=ds.readBoolLV();
          divertActiveCapacityFlag=true;
        }break;
        case divertModifiableTag:
        {
          if(divertModifiableFlag)
          {
            throw protogen::framework::DuplicateFieldException("divertModifiable");
          }
          divertModifiable=ds.readBoolLV();
          divertModifiableFlag=true;
        }break;
        case udhConcatTag:
        {
          if(udhConcatFlag)
          {
            throw protogen::framework::DuplicateFieldException("udhConcat");
          }
          udhConcat=ds.readBoolLV();
          udhConcatFlag=true;
        }break;
        case translitTag:
        {
          if(translitFlag)
          {
            throw protogen::framework::DuplicateFieldException("translit");
          }
          translit=ds.readBoolLV();
          translitFlag=true;
        }break;
        case closedGroupIdTag:
        {
          if(closedGroupIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("closedGroupId");
          }
          closedGroupId=ds.readInt32LV();
          closedGroupIdFlag=true;
        }break;
        case accessMaskInTag:
        {
          if(accessMaskInFlag)
          {
            throw protogen::framework::DuplicateFieldException("accessMaskIn");
          }
          accessMaskIn=ds.readInt32LV();
          accessMaskInFlag=true;
        }break;
        case accessMaskOutTag:
        {
          if(accessMaskOutFlag)
          {
            throw protogen::framework::DuplicateFieldException("accessMaskOut");
          }
          accessMaskOut=ds.readInt32LV();
          accessMaskOutFlag=true;
        }break;
        case subscriptionTag:
        {
          if(subscriptionFlag)
          {
            throw protogen::framework::DuplicateFieldException("subscription");
          }
          subscription=ds.readInt32LV();
          subscriptionFlag=true;
        }break;
        case sponsoredTag:
        {
          if(sponsoredFlag)
          {
            throw protogen::framework::DuplicateFieldException("sponsored");
          }
          sponsored=ds.readByteLV();
          sponsoredFlag=true;
        }break;
        case nickTag:
        {
          if(nickFlag)
          {
            throw protogen::framework::DuplicateFieldException("nick");
          }
          nick=ds.readStrLV();
          nickFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("Profile",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!divertFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divert");
    }
    if(!localeFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("locale");
    }
    if(!codepageFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("codepage");
    }
    if(!reportOptionsFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("reportOptions");
    }
    if(!hideFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("hide");
    }
    if(!hideModifiableFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("hideModifiable");
    }
    if(!divertActiveFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActive");
    }
    if(!divertActiveAbsentFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveAbsent");
    }
    if(!divertActiveBlockedFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveBlocked");
    }
    if(!divertActiveBarredFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveBarred");
    }
    if(!divertActiveCapacityFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertActiveCapacity");
    }
    if(!divertModifiableFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("divertModifiable");
    }
    if(!udhConcatFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("udhConcat");
    }
    if(!translitFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("translit");
    }
    if(!closedGroupIdFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("closedGroupId");
    }
    if(!accessMaskInFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("accessMaskIn");
    }
    if(!accessMaskOutFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("accessMaskOut");
    }

  }


protected:
  //static const uint8_t versionMajor=1;
  //static const uint8_t versionMinor=0;

  static const uint32_t divertTag=1;
  static const uint32_t localeTag=2;
  static const uint32_t codepageTag=3;
  static const uint32_t reportOptionsTag=4;
  static const uint32_t hideTag=5;
  static const uint32_t hideModifiableTag=6;
  static const uint32_t divertActiveTag=7;
  static const uint32_t divertActiveAbsentTag=8;
  static const uint32_t divertActiveBlockedTag=9;
  static const uint32_t divertActiveBarredTag=10;
  static const uint32_t divertActiveCapacityTag=11;
  static const uint32_t divertModifiableTag=12;
  static const uint32_t udhConcatTag=13;
  static const uint32_t translitTag=14;
  static const uint32_t closedGroupIdTag=15;
  static const uint32_t accessMaskInTag=16;
  static const uint32_t accessMaskOutTag=17;
  static const uint32_t subscriptionTag=18;
  static const uint32_t sponsoredTag=19;
  static const uint32_t nickTag=20;


  std::string divert;
  std::string locale;
  uint8_t codepage;
  uint8_t reportOptions;
  uint8_t hide;
  bool hideModifiable;
  bool divertActive;
  bool divertActiveAbsent;
  bool divertActiveBlocked;
  bool divertActiveBarred;
  bool divertActiveCapacity;
  bool divertModifiable;
  bool udhConcat;
  bool translit;
  uint32_t closedGroupId;
  uint32_t accessMaskIn;
  uint32_t accessMaskOut;
  uint32_t subscription;
  uint8_t sponsored;
  std::string nick;

  bool divertFlag;
  bool localeFlag;
  bool codepageFlag;
  bool reportOptionsFlag;
  bool hideFlag;
  bool hideModifiableFlag;
  bool divertActiveFlag;
  bool divertActiveAbsentFlag;
  bool divertActiveBlockedFlag;
  bool divertActiveBarredFlag;
  bool divertActiveCapacityFlag;
  bool divertModifiableFlag;
  bool udhConcatFlag;
  bool translitFlag;
  bool closedGroupIdFlag;
  bool accessMaskInFlag;
  bool accessMaskOutFlag;
  bool subscriptionFlag;
  bool sponsoredFlag;
  bool nickFlag;
};

}
}
}
}
}
#endif
