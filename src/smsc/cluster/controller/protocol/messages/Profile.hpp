#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_PROFILE_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_PROFILE_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "ProfileCharset.hpp"
#include "ReportOptions.hpp"
#include "HideOptions.hpp"


#ident "@(#) Profile version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


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
      rv+=ProfileCharset::getNameByValue(codepage);
    }
    if(reportOptionsFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="reportOptions=";
      rv+=ReportOptions::getNameByValue(reportOptions);
    }
    if(hideFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="hide=";
      rv+=HideOptions::getNameByValue(hide);
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
      sprintf(buf,"%d",closedGroupId);
      rv+=buf;
    }
    if(accessMaskInFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="accessMaskIn=";
      sprintf(buf,"%d",accessMaskIn);
      rv+=buf;
    }
    if(accessMaskOutFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="accessMaskOut=";
      sprintf(buf,"%d",accessMaskOut);
      rv+=buf;
    }
    if(subscriptionFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="subscription=";
      sprintf(buf,"%d",subscription);
      rv+=buf;
    }
    if(sponsoredFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="sponsored=";
      sprintf(buf,"%d",(int)sponsored);
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
  int32_t length()const
  {
    int32_t rv=0;
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
      throw eyeline::protogen::framework::FieldIsNullException("divert");
    }
    return divert;
  }
  void setDivert(const std::string& argValue)
  {
    divert=argValue;
    divertFlag=true;
  }
  std::string& getDivertRef()
  {
    divertFlag=true;
    return divert;
  }
  bool hasDivert()const
  {
    return divertFlag;
  }
  const std::string& getLocale()const
  {
    if(!localeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("locale");
    }
    return locale;
  }
  void setLocale(const std::string& argValue)
  {
    locale=argValue;
    localeFlag=true;
  }
  std::string& getLocaleRef()
  {
    localeFlag=true;
    return locale;
  }
  bool hasLocale()const
  {
    return localeFlag;
  }
  const ProfileCharset::type& getCodepage()const
  {
    if(!codepageFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("codepage");
    }
    return codepage;
  }
  void setCodepage(const ProfileCharset::type& argValue)
  {
    if(!ProfileCharset::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ProfileCharset",argValue);
    }
    codepage=argValue;
    codepageFlag=true;
  }
  ProfileCharset::type& getCodepageRef()
  {
    codepageFlag=true;
    return codepage;
  }
  bool hasCodepage()const
  {
    return codepageFlag;
  }
  const ReportOptions::type& getReportOptions()const
  {
    if(!reportOptionsFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("reportOptions");
    }
    return reportOptions;
  }
  void setReportOptions(const ReportOptions::type& argValue)
  {
    if(!ReportOptions::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ReportOptions",argValue);
    }
    reportOptions=argValue;
    reportOptionsFlag=true;
  }
  ReportOptions::type& getReportOptionsRef()
  {
    reportOptionsFlag=true;
    return reportOptions;
  }
  bool hasReportOptions()const
  {
    return reportOptionsFlag;
  }
  const HideOptions::type& getHide()const
  {
    if(!hideFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("hide");
    }
    return hide;
  }
  void setHide(const HideOptions::type& argValue)
  {
    if(!HideOptions::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("HideOptions",argValue);
    }
    hide=argValue;
    hideFlag=true;
  }
  HideOptions::type& getHideRef()
  {
    hideFlag=true;
    return hide;
  }
  bool hasHide()const
  {
    return hideFlag;
  }
  bool getHideModifiable()const
  {
    if(!hideModifiableFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("hideModifiable");
    }
    return hideModifiable;
  }
  void setHideModifiable(bool argValue)
  {
    hideModifiable=argValue;
    hideModifiableFlag=true;
  }
  bool& getHideModifiableRef()
  {
    hideModifiableFlag=true;
    return hideModifiable;
  }
  bool hasHideModifiable()const
  {
    return hideModifiableFlag;
  }
  bool getDivertActive()const
  {
    if(!divertActiveFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("divertActive");
    }
    return divertActive;
  }
  void setDivertActive(bool argValue)
  {
    divertActive=argValue;
    divertActiveFlag=true;
  }
  bool& getDivertActiveRef()
  {
    divertActiveFlag=true;
    return divertActive;
  }
  bool hasDivertActive()const
  {
    return divertActiveFlag;
  }
  bool getDivertActiveAbsent()const
  {
    if(!divertActiveAbsentFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("divertActiveAbsent");
    }
    return divertActiveAbsent;
  }
  void setDivertActiveAbsent(bool argValue)
  {
    divertActiveAbsent=argValue;
    divertActiveAbsentFlag=true;
  }
  bool& getDivertActiveAbsentRef()
  {
    divertActiveAbsentFlag=true;
    return divertActiveAbsent;
  }
  bool hasDivertActiveAbsent()const
  {
    return divertActiveAbsentFlag;
  }
  bool getDivertActiveBlocked()const
  {
    if(!divertActiveBlockedFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("divertActiveBlocked");
    }
    return divertActiveBlocked;
  }
  void setDivertActiveBlocked(bool argValue)
  {
    divertActiveBlocked=argValue;
    divertActiveBlockedFlag=true;
  }
  bool& getDivertActiveBlockedRef()
  {
    divertActiveBlockedFlag=true;
    return divertActiveBlocked;
  }
  bool hasDivertActiveBlocked()const
  {
    return divertActiveBlockedFlag;
  }
  bool getDivertActiveBarred()const
  {
    if(!divertActiveBarredFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("divertActiveBarred");
    }
    return divertActiveBarred;
  }
  void setDivertActiveBarred(bool argValue)
  {
    divertActiveBarred=argValue;
    divertActiveBarredFlag=true;
  }
  bool& getDivertActiveBarredRef()
  {
    divertActiveBarredFlag=true;
    return divertActiveBarred;
  }
  bool hasDivertActiveBarred()const
  {
    return divertActiveBarredFlag;
  }
  bool getDivertActiveCapacity()const
  {
    if(!divertActiveCapacityFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("divertActiveCapacity");
    }
    return divertActiveCapacity;
  }
  void setDivertActiveCapacity(bool argValue)
  {
    divertActiveCapacity=argValue;
    divertActiveCapacityFlag=true;
  }
  bool& getDivertActiveCapacityRef()
  {
    divertActiveCapacityFlag=true;
    return divertActiveCapacity;
  }
  bool hasDivertActiveCapacity()const
  {
    return divertActiveCapacityFlag;
  }
  bool getDivertModifiable()const
  {
    if(!divertModifiableFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("divertModifiable");
    }
    return divertModifiable;
  }
  void setDivertModifiable(bool argValue)
  {
    divertModifiable=argValue;
    divertModifiableFlag=true;
  }
  bool& getDivertModifiableRef()
  {
    divertModifiableFlag=true;
    return divertModifiable;
  }
  bool hasDivertModifiable()const
  {
    return divertModifiableFlag;
  }
  bool getUdhConcat()const
  {
    if(!udhConcatFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("udhConcat");
    }
    return udhConcat;
  }
  void setUdhConcat(bool argValue)
  {
    udhConcat=argValue;
    udhConcatFlag=true;
  }
  bool& getUdhConcatRef()
  {
    udhConcatFlag=true;
    return udhConcat;
  }
  bool hasUdhConcat()const
  {
    return udhConcatFlag;
  }
  bool getTranslit()const
  {
    if(!translitFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("translit");
    }
    return translit;
  }
  void setTranslit(bool argValue)
  {
    translit=argValue;
    translitFlag=true;
  }
  bool& getTranslitRef()
  {
    translitFlag=true;
    return translit;
  }
  bool hasTranslit()const
  {
    return translitFlag;
  }
  int32_t getClosedGroupId()const
  {
    if(!closedGroupIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("closedGroupId");
    }
    return closedGroupId;
  }
  void setClosedGroupId(int32_t argValue)
  {
    closedGroupId=argValue;
    closedGroupIdFlag=true;
  }
  int32_t& getClosedGroupIdRef()
  {
    closedGroupIdFlag=true;
    return closedGroupId;
  }
  bool hasClosedGroupId()const
  {
    return closedGroupIdFlag;
  }
  int32_t getAccessMaskIn()const
  {
    if(!accessMaskInFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("accessMaskIn");
    }
    return accessMaskIn;
  }
  void setAccessMaskIn(int32_t argValue)
  {
    accessMaskIn=argValue;
    accessMaskInFlag=true;
  }
  int32_t& getAccessMaskInRef()
  {
    accessMaskInFlag=true;
    return accessMaskIn;
  }
  bool hasAccessMaskIn()const
  {
    return accessMaskInFlag;
  }
  int32_t getAccessMaskOut()const
  {
    if(!accessMaskOutFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("accessMaskOut");
    }
    return accessMaskOut;
  }
  void setAccessMaskOut(int32_t argValue)
  {
    accessMaskOut=argValue;
    accessMaskOutFlag=true;
  }
  int32_t& getAccessMaskOutRef()
  {
    accessMaskOutFlag=true;
    return accessMaskOut;
  }
  bool hasAccessMaskOut()const
  {
    return accessMaskOutFlag;
  }
  int32_t getSubscription()const
  {
    if(!subscriptionFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("subscription");
    }
    return subscription;
  }
  void setSubscription(int32_t argValue)
  {
    subscription=argValue;
    subscriptionFlag=true;
  }
  int32_t& getSubscriptionRef()
  {
    subscriptionFlag=true;
    return subscription;
  }
  bool hasSubscription()const
  {
    return subscriptionFlag;
  }
  int8_t getSponsored()const
  {
    if(!sponsoredFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("sponsored");
    }
    return sponsored;
  }
  void setSponsored(int8_t argValue)
  {
    sponsored=argValue;
    sponsoredFlag=true;
  }
  int8_t& getSponsoredRef()
  {
    sponsoredFlag=true;
    return sponsored;
  }
  bool hasSponsored()const
  {
    return sponsoredFlag;
  }
  const std::string& getNick()const
  {
    if(!nickFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("nick");
    }
    return nick;
  }
  void setNick(const std::string& argValue)
  {
    nick=argValue;
    nickFlag=true;
  }
  std::string& getNickRef()
  {
    nickFlag=true;
    return nick;
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divert");
    }
    if(!localeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("locale");
    }
    if(!codepageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("codepage");
    }
    if(!reportOptionsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("reportOptions");
    }
    if(!hideFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("hide");
    }
    if(!hideModifiableFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("hideModifiable");
    }
    if(!divertActiveFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActive");
    }
    if(!divertActiveAbsentFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveAbsent");
    }
    if(!divertActiveBlockedFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveBlocked");
    }
    if(!divertActiveBarredFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveBarred");
    }
    if(!divertActiveCapacityFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveCapacity");
    }
    if(!divertModifiableFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertModifiable");
    }
    if(!udhConcatFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("udhConcat");
    }
    if(!translitFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("translit");
    }
    if(!closedGroupIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("closedGroupId");
    }
    if(!accessMaskInFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("accessMaskIn");
    }
    if(!accessMaskOutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("accessMaskOut");
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
    //int8_t rdVersionMajor=ds.readByte();
    //int8_t rdVersionMinor=ds.readByte();
    //if(rdVersionMajor!=versionMajor)
    //{
    //  throw protogen::framework::IncompatibleVersionException("Profile");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
      switch(tag)
      {
        case divertTag:
        {
          if(divertFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divert");
          }
          divert=ds.readStrLV();
          divertFlag=true;
        }break;
        case localeTag:
        {
          if(localeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("locale");
          }
          locale=ds.readStrLV();
          localeFlag=true;
        }break;
        case codepageTag:
        {
          if(codepageFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("codepage");
          }
          codepage=ds.readByteLV();
          codepageFlag=true;
        }break;
        case reportOptionsTag:
        {
          if(reportOptionsFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("reportOptions");
          }
          reportOptions=ds.readByteLV();
          reportOptionsFlag=true;
        }break;
        case hideTag:
        {
          if(hideFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("hide");
          }
          hide=ds.readByteLV();
          hideFlag=true;
        }break;
        case hideModifiableTag:
        {
          if(hideModifiableFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("hideModifiable");
          }
          hideModifiable=ds.readBoolLV();
          hideModifiableFlag=true;
        }break;
        case divertActiveTag:
        {
          if(divertActiveFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divertActive");
          }
          divertActive=ds.readBoolLV();
          divertActiveFlag=true;
        }break;
        case divertActiveAbsentTag:
        {
          if(divertActiveAbsentFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divertActiveAbsent");
          }
          divertActiveAbsent=ds.readBoolLV();
          divertActiveAbsentFlag=true;
        }break;
        case divertActiveBlockedTag:
        {
          if(divertActiveBlockedFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divertActiveBlocked");
          }
          divertActiveBlocked=ds.readBoolLV();
          divertActiveBlockedFlag=true;
        }break;
        case divertActiveBarredTag:
        {
          if(divertActiveBarredFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divertActiveBarred");
          }
          divertActiveBarred=ds.readBoolLV();
          divertActiveBarredFlag=true;
        }break;
        case divertActiveCapacityTag:
        {
          if(divertActiveCapacityFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divertActiveCapacity");
          }
          divertActiveCapacity=ds.readBoolLV();
          divertActiveCapacityFlag=true;
        }break;
        case divertModifiableTag:
        {
          if(divertModifiableFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("divertModifiable");
          }
          divertModifiable=ds.readBoolLV();
          divertModifiableFlag=true;
        }break;
        case udhConcatTag:
        {
          if(udhConcatFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("udhConcat");
          }
          udhConcat=ds.readBoolLV();
          udhConcatFlag=true;
        }break;
        case translitTag:
        {
          if(translitFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("translit");
          }
          translit=ds.readBoolLV();
          translitFlag=true;
        }break;
        case closedGroupIdTag:
        {
          if(closedGroupIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("closedGroupId");
          }
          closedGroupId=ds.readInt32LV();
          closedGroupIdFlag=true;
        }break;
        case accessMaskInTag:
        {
          if(accessMaskInFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("accessMaskIn");
          }
          accessMaskIn=ds.readInt32LV();
          accessMaskInFlag=true;
        }break;
        case accessMaskOutTag:
        {
          if(accessMaskOutFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("accessMaskOut");
          }
          accessMaskOut=ds.readInt32LV();
          accessMaskOutFlag=true;
        }break;
        case subscriptionTag:
        {
          if(subscriptionFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("subscription");
          }
          subscription=ds.readInt32LV();
          subscriptionFlag=true;
        }break;
        case sponsoredTag:
        {
          if(sponsoredFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("sponsored");
          }
          sponsored=ds.readByteLV();
          sponsoredFlag=true;
        }break;
        case nickTag:
        {
          if(nickFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("nick");
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
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divert");
    }
    if(!localeFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("locale");
    }
    if(!codepageFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("codepage");
    }
    if(!reportOptionsFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("reportOptions");
    }
    if(!hideFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("hide");
    }
    if(!hideModifiableFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("hideModifiable");
    }
    if(!divertActiveFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActive");
    }
    if(!divertActiveAbsentFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveAbsent");
    }
    if(!divertActiveBlockedFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveBlocked");
    }
    if(!divertActiveBarredFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveBarred");
    }
    if(!divertActiveCapacityFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertActiveCapacity");
    }
    if(!divertModifiableFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("divertModifiable");
    }
    if(!udhConcatFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("udhConcat");
    }
    if(!translitFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("translit");
    }
    if(!closedGroupIdFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("closedGroupId");
    }
    if(!accessMaskInFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("accessMaskIn");
    }
    if(!accessMaskOutFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("accessMaskOut");
    }

  }


 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t divertTag=1;
  static const int32_t localeTag=2;
  static const int32_t codepageTag=3;
  static const int32_t reportOptionsTag=4;
  static const int32_t hideTag=5;
  static const int32_t hideModifiableTag=6;
  static const int32_t divertActiveTag=7;
  static const int32_t divertActiveAbsentTag=8;
  static const int32_t divertActiveBlockedTag=9;
  static const int32_t divertActiveBarredTag=10;
  static const int32_t divertActiveCapacityTag=11;
  static const int32_t divertModifiableTag=12;
  static const int32_t udhConcatTag=13;
  static const int32_t translitTag=14;
  static const int32_t closedGroupIdTag=15;
  static const int32_t accessMaskInTag=16;
  static const int32_t accessMaskOutTag=17;
  static const int32_t subscriptionTag=18;
  static const int32_t sponsoredTag=19;
  static const int32_t nickTag=20;


  std::string divert;
  std::string locale;
  ProfileCharset::type codepage;
  ReportOptions::type reportOptions;
  HideOptions::type hide;
  bool hideModifiable;
  bool divertActive;
  bool divertActiveAbsent;
  bool divertActiveBlocked;
  bool divertActiveBarred;
  bool divertActiveCapacity;
  bool divertModifiable;
  bool udhConcat;
  bool translit;
  int32_t closedGroupId;
  int32_t accessMaskIn;
  int32_t accessMaskOut;
  int32_t subscription;
  int8_t sponsored;
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
