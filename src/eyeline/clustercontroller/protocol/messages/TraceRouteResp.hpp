#ifndef __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_TRACEROUTERESP_HPP__
#define __GENERATED_MESSAGE_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_TRACEROUTERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"
#include "RouteFindInfo.hpp"
#include "BillingMode.hpp"


#ident "@(#) TraceRouteResp version 1.0"



namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class TraceRouteResp{
public:
  TraceRouteResp()
  {
    Clear();
  }
  void Clear()
  {
    seqNum=0;
    respFlag=false;
    aliasInfoFlag=false;
    foundInfoFlag=false;
    routeIdFlag=false;
    srcAddrFlag=false;
    dstAddrFlag=false;
    smeIdFlag=false;
    srcSmeIdFlag=false;
    priorityFlag=false;
    serviceIdFlag=false;
    deliveryModeFlag=false;
    forwardToFlag=false;
    billingFlag=false;
    archivingFlag=false;
    enabledFlag=false;
    suppressDlvRepFlag=false;
    traceFlag=false;
    trace.clear();
  }
 
  static int32_t getTag()
  {
    return 1008;
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
    if(aliasInfoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="aliasInfo=";
      rv+=aliasInfo;
    }
    if(foundInfoFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="foundInfo=";
      rv+=RouteFindInfo::getNameByValue(foundInfo);
    }
    if(routeIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="routeId=";
      rv+=routeId;
    }
    if(srcAddrFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="srcAddr=";
      rv+=srcAddr;
    }
    if(dstAddrFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="dstAddr=";
      rv+=dstAddr;
    }
    if(smeIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="smeId=";
      rv+=smeId;
    }
    if(srcSmeIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="srcSmeId=";
      rv+=srcSmeId;
    }
    if(priorityFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="priority=";
      sprintf(buf,"%d",priority);
      rv+=buf;
    }
    if(serviceIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="serviceId=";
      sprintf(buf,"%d",serviceId);
      rv+=buf;
    }
    if(deliveryModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryMode=";
      sprintf(buf,"%d",(int)deliveryMode);
      rv+=buf;
    }
    if(forwardToFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="forwardTo=";
      rv+=forwardTo;
    }
    if(billingFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="billing=";
      rv+=BillingMode::getNameByValue(billing);
    }
    if(archivingFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="archiving=";
      rv+=archiving?"true":"false";
    }
    if(enabledFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="enabled=";
      rv+=enabled?"true":"false";
    }
    if(suppressDlvRepFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="suppressDlvRep=";
      rv+=suppressDlvRep?"true":"false";
    }
    if(traceFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="trace=";
      rv+="[";
      bool first=true;
      for(std::vector<std::string>::const_iterator it=trace.begin(),end=trace.end();it!=end;it++)
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
    if(respFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=resp.length<DataStream>();
    }
    if(aliasInfoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(aliasInfo);
    }
    if(foundInfoFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(foundInfo);
    }
    if(routeIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(routeId);
    }
    if(srcAddrFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(srcAddr);
    }
    if(dstAddrFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(dstAddr);
    }
    if(smeIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(smeId);
    }
    if(srcSmeIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(srcSmeId);
    }
    if(priorityFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(priority);
    }
    if(serviceIdFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(serviceId);
    }
    if(deliveryModeFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(deliveryMode);
    }
    if(forwardToFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(forwardTo);
    }
    if(billingFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(billing);
    }
    if(archivingFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(archiving);
    }
    if(enabledFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(enabled);
    }
    if(suppressDlvRepFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(suppressDlvRep);
    }
    if(traceFlag)
    {
      rv+=DataStream::tagTypeSize;
      rv+=DataStream::lengthTypeSize;
      rv+=DataStream::fieldSize(trace);
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
  const std::string& getAliasInfo()const
  {
    if(!aliasInfoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("aliasInfo");
    }
    return aliasInfo;
  }
  void setAliasInfo(const std::string& argValue)
  {
    aliasInfo=argValue;
    aliasInfoFlag=true;
  }
  std::string& getAliasInfoRef()
  {
    aliasInfoFlag=true;
    return aliasInfo;
  }
  bool hasAliasInfo()const
  {
    return aliasInfoFlag;
  }
  const RouteFindInfo::type& getFoundInfo()const
  {
    if(!foundInfoFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("foundInfo");
    }
    return foundInfo;
  }
  void setFoundInfo(const RouteFindInfo::type& argValue)
  {
    if(!RouteFindInfo::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("RouteFindInfo",argValue);
    }
    foundInfo=argValue;
    foundInfoFlag=true;
  }
  RouteFindInfo::type& getFoundInfoRef()
  {
    foundInfoFlag=true;
    return foundInfo;
  }
  bool hasFoundInfo()const
  {
    return foundInfoFlag;
  }
  const std::string& getRouteId()const
  {
    if(!routeIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("routeId");
    }
    return routeId;
  }
  void setRouteId(const std::string& argValue)
  {
    routeId=argValue;
    routeIdFlag=true;
  }
  std::string& getRouteIdRef()
  {
    routeIdFlag=true;
    return routeId;
  }
  bool hasRouteId()const
  {
    return routeIdFlag;
  }
  const std::string& getSrcAddr()const
  {
    if(!srcAddrFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("srcAddr");
    }
    return srcAddr;
  }
  void setSrcAddr(const std::string& argValue)
  {
    srcAddr=argValue;
    srcAddrFlag=true;
  }
  std::string& getSrcAddrRef()
  {
    srcAddrFlag=true;
    return srcAddr;
  }
  bool hasSrcAddr()const
  {
    return srcAddrFlag;
  }
  const std::string& getDstAddr()const
  {
    if(!dstAddrFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("dstAddr");
    }
    return dstAddr;
  }
  void setDstAddr(const std::string& argValue)
  {
    dstAddr=argValue;
    dstAddrFlag=true;
  }
  std::string& getDstAddrRef()
  {
    dstAddrFlag=true;
    return dstAddr;
  }
  bool hasDstAddr()const
  {
    return dstAddrFlag;
  }
  const std::string& getSmeId()const
  {
    if(!smeIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("smeId");
    }
    return smeId;
  }
  void setSmeId(const std::string& argValue)
  {
    smeId=argValue;
    smeIdFlag=true;
  }
  std::string& getSmeIdRef()
  {
    smeIdFlag=true;
    return smeId;
  }
  bool hasSmeId()const
  {
    return smeIdFlag;
  }
  const std::string& getSrcSmeId()const
  {
    if(!srcSmeIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("srcSmeId");
    }
    return srcSmeId;
  }
  void setSrcSmeId(const std::string& argValue)
  {
    srcSmeId=argValue;
    srcSmeIdFlag=true;
  }
  std::string& getSrcSmeIdRef()
  {
    srcSmeIdFlag=true;
    return srcSmeId;
  }
  bool hasSrcSmeId()const
  {
    return srcSmeIdFlag;
  }
  int32_t getPriority()const
  {
    if(!priorityFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("priority");
    }
    return priority;
  }
  void setPriority(int32_t argValue)
  {
    priority=argValue;
    priorityFlag=true;
  }
  int32_t& getPriorityRef()
  {
    priorityFlag=true;
    return priority;
  }
  bool hasPriority()const
  {
    return priorityFlag;
  }
  int32_t getServiceId()const
  {
    if(!serviceIdFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("serviceId");
    }
    return serviceId;
  }
  void setServiceId(int32_t argValue)
  {
    serviceId=argValue;
    serviceIdFlag=true;
  }
  int32_t& getServiceIdRef()
  {
    serviceIdFlag=true;
    return serviceId;
  }
  bool hasServiceId()const
  {
    return serviceIdFlag;
  }
  int8_t getDeliveryMode()const
  {
    if(!deliveryModeFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("deliveryMode");
    }
    return deliveryMode;
  }
  void setDeliveryMode(int8_t argValue)
  {
    deliveryMode=argValue;
    deliveryModeFlag=true;
  }
  int8_t& getDeliveryModeRef()
  {
    deliveryModeFlag=true;
    return deliveryMode;
  }
  bool hasDeliveryMode()const
  {
    return deliveryModeFlag;
  }
  const std::string& getForwardTo()const
  {
    if(!forwardToFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("forwardTo");
    }
    return forwardTo;
  }
  void setForwardTo(const std::string& argValue)
  {
    forwardTo=argValue;
    forwardToFlag=true;
  }
  std::string& getForwardToRef()
  {
    forwardToFlag=true;
    return forwardTo;
  }
  bool hasForwardTo()const
  {
    return forwardToFlag;
  }
  const BillingMode::type& getBilling()const
  {
    if(!billingFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("billing");
    }
    return billing;
  }
  void setBilling(const BillingMode::type& argValue)
  {
    if(!BillingMode::isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("BillingMode",argValue);
    }
    billing=argValue;
    billingFlag=true;
  }
  BillingMode::type& getBillingRef()
  {
    billingFlag=true;
    return billing;
  }
  bool hasBilling()const
  {
    return billingFlag;
  }
  bool getArchiving()const
  {
    if(!archivingFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("archiving");
    }
    return archiving;
  }
  void setArchiving(bool argValue)
  {
    archiving=argValue;
    archivingFlag=true;
  }
  bool& getArchivingRef()
  {
    archivingFlag=true;
    return archiving;
  }
  bool hasArchiving()const
  {
    return archivingFlag;
  }
  bool getEnabled()const
  {
    if(!enabledFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("enabled");
    }
    return enabled;
  }
  void setEnabled(bool argValue)
  {
    enabled=argValue;
    enabledFlag=true;
  }
  bool& getEnabledRef()
  {
    enabledFlag=true;
    return enabled;
  }
  bool hasEnabled()const
  {
    return enabledFlag;
  }
  bool getSuppressDlvRep()const
  {
    if(!suppressDlvRepFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("suppressDlvRep");
    }
    return suppressDlvRep;
  }
  void setSuppressDlvRep(bool argValue)
  {
    suppressDlvRep=argValue;
    suppressDlvRepFlag=true;
  }
  bool& getSuppressDlvRepRef()
  {
    suppressDlvRepFlag=true;
    return suppressDlvRep;
  }
  bool hasSuppressDlvRep()const
  {
    return suppressDlvRepFlag;
  }
  const std::vector<std::string>& getTrace()const
  {
    if(!traceFlag)
    {
      throw eyeline::protogen::framework::FieldIsNullException("trace");
    }
    return trace;
  }
  void setTrace(const std::vector<std::string>& argValue)
  {
    trace=argValue;
    traceFlag=true;
  }
  std::vector<std::string>& getTraceRef()
  {
    traceFlag=true;
    return trace;
  }
  bool hasTrace()const
  {
    return traceFlag;
  }
  template <class DataStream>
  void serialize(DataStream& ds)const
  {
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!aliasInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("aliasInfo");
    }
    if(!foundInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("foundInfo");
    }
    //ds.writeByte(versionMajor);
    //ds.writeByte(versionMinor);
    //ds.writeInt32(seqNum);
    ds.writeTag(respTag);
    ds.writeLength(resp.length<DataStream>());
    resp.serialize(ds);
    ds.writeTag(aliasInfoTag);
    ds.writeStrLV(aliasInfo);
    ds.writeTag(foundInfoTag);
    ds.writeByteLV(foundInfo);
 
    if(routeIdFlag)
    {
      ds.writeTag(routeIdTag);
    ds.writeStrLV(routeId);
    }
    if(srcAddrFlag)
    {
      ds.writeTag(srcAddrTag);
    ds.writeStrLV(srcAddr);
    }
    if(dstAddrFlag)
    {
      ds.writeTag(dstAddrTag);
    ds.writeStrLV(dstAddr);
    }
    if(smeIdFlag)
    {
      ds.writeTag(smeIdTag);
    ds.writeStrLV(smeId);
    }
    if(srcSmeIdFlag)
    {
      ds.writeTag(srcSmeIdTag);
    ds.writeStrLV(srcSmeId);
    }
    if(priorityFlag)
    {
      ds.writeTag(priorityTag);
    ds.writeInt32LV(priority);
    }
    if(serviceIdFlag)
    {
      ds.writeTag(serviceIdTag);
    ds.writeInt32LV(serviceId);
    }
    if(deliveryModeFlag)
    {
      ds.writeTag(deliveryModeTag);
    ds.writeByteLV(deliveryMode);
    }
    if(forwardToFlag)
    {
      ds.writeTag(forwardToTag);
    ds.writeStrLV(forwardTo);
    }
    if(billingFlag)
    {
      ds.writeTag(billingTag);
    ds.writeByteLV(billing);
 
    }
    if(archivingFlag)
    {
      ds.writeTag(archivingTag);
    ds.writeBoolLV(archiving);
    }
    if(enabledFlag)
    {
      ds.writeTag(enabledTag);
    ds.writeBoolLV(enabled);
    }
    if(suppressDlvRepFlag)
    {
      ds.writeTag(suppressDlvRepTag);
    ds.writeBoolLV(suppressDlvRep);
    }
    if(traceFlag)
    {
      ds.writeTag(traceTag);
    ds.writeLength(DataStream::fieldSize(trace));
    for(std::vector<std::string>::const_iterator it=trace.begin(),end=trace.end();it!=end;it++)
    {
      ds.writeStr(*it);
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
    //  throw protogen::framework::IncompatibleVersionException("TraceRouteResp");
    //}
    //seqNum=ds.readInt32();
    while(!endOfMessage)
    {
      DataStream::TagType tag=ds.readTag();
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
        case aliasInfoTag:
        {
          if(aliasInfoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("aliasInfo");
          }
          aliasInfo=ds.readStrLV();
          aliasInfoFlag=true;
        }break;
        case foundInfoTag:
        {
          if(foundInfoFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("foundInfo");
          }
          foundInfo=ds.readByteLV();
          foundInfoFlag=true;
        }break;
        case routeIdTag:
        {
          if(routeIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("routeId");
          }
          routeId=ds.readStrLV();
          routeIdFlag=true;
        }break;
        case srcAddrTag:
        {
          if(srcAddrFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("srcAddr");
          }
          srcAddr=ds.readStrLV();
          srcAddrFlag=true;
        }break;
        case dstAddrTag:
        {
          if(dstAddrFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("dstAddr");
          }
          dstAddr=ds.readStrLV();
          dstAddrFlag=true;
        }break;
        case smeIdTag:
        {
          if(smeIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("smeId");
          }
          smeId=ds.readStrLV();
          smeIdFlag=true;
        }break;
        case srcSmeIdTag:
        {
          if(srcSmeIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("srcSmeId");
          }
          srcSmeId=ds.readStrLV();
          srcSmeIdFlag=true;
        }break;
        case priorityTag:
        {
          if(priorityFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("priority");
          }
          priority=ds.readInt32LV();
          priorityFlag=true;
        }break;
        case serviceIdTag:
        {
          if(serviceIdFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("serviceId");
          }
          serviceId=ds.readInt32LV();
          serviceIdFlag=true;
        }break;
        case deliveryModeTag:
        {
          if(deliveryModeFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("deliveryMode");
          }
          deliveryMode=ds.readByteLV();
          deliveryModeFlag=true;
        }break;
        case forwardToTag:
        {
          if(forwardToFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("forwardTo");
          }
          forwardTo=ds.readStrLV();
          forwardToFlag=true;
        }break;
        case billingTag:
        {
          if(billingFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("billing");
          }
          billing=ds.readByteLV();
          billingFlag=true;
        }break;
        case archivingTag:
        {
          if(archivingFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("archiving");
          }
          archiving=ds.readBoolLV();
          archivingFlag=true;
        }break;
        case enabledTag:
        {
          if(enabledFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("enabled");
          }
          enabled=ds.readBoolLV();
          enabledFlag=true;
        }break;
        case suppressDlvRepTag:
        {
          if(suppressDlvRepFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("suppressDlvRep");
          }
          suppressDlvRep=ds.readBoolLV();
          suppressDlvRepFlag=true;
        }break;
        case traceTag:
        {
          if(traceFlag)
          {
            throw eyeline::protogen::framework::DuplicateFieldException("trace");
          }
          typename DataStream::LengthType len=ds.readLength(),rd=0;
          while(rd<len)
          {
            trace.push_back(ds.readStr());
            rd+=DataStream::fieldSize(trace.back());
          }
          traceFlag=true;
        }break;
        case DataStream::endOfMessage_tag:
          endOfMessage=true;
          break;
        default:
          //if(rdVersionMinor==versionMinor)
          //{
          //  throw protogen::framework::UnexpectedTag("TraceRouteResp",tag);
          //}
          ds.skip(ds.readLength());
      }
    }
    if(!respFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!aliasInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("aliasInfo");
    }
    if(!foundInfoFlag)
    {
      throw eyeline::protogen::framework::MandatoryFieldMissingException("foundInfo");
    }

  }

  int32_t getSeqNum()const
  {
    return seqNum;
  }

  void setSeqNum(int32_t argValue)
  {
    seqNum=argValue;
  }

 

protected:
  //static const int8_t versionMajor=1;
  //static const int8_t versionMinor=0;

  static const int32_t respTag=1;
  static const int32_t aliasInfoTag=2;
  static const int32_t foundInfoTag=3;
  static const int32_t routeIdTag=4;
  static const int32_t srcAddrTag=5;
  static const int32_t dstAddrTag=6;
  static const int32_t smeIdTag=7;
  static const int32_t srcSmeIdTag=8;
  static const int32_t priorityTag=9;
  static const int32_t serviceIdTag=10;
  static const int32_t deliveryModeTag=11;
  static const int32_t forwardToTag=12;
  static const int32_t billingTag=13;
  static const int32_t archivingTag=14;
  static const int32_t enabledTag=15;
  static const int32_t suppressDlvRepTag=16;
  static const int32_t traceTag=17;

  int32_t seqNum;

  Response resp;
  std::string aliasInfo;
  RouteFindInfo::type foundInfo;
  std::string routeId;
  std::string srcAddr;
  std::string dstAddr;
  std::string smeId;
  std::string srcSmeId;
  int32_t priority;
  int32_t serviceId;
  int8_t deliveryMode;
  std::string forwardTo;
  BillingMode::type billing;
  bool archiving;
  bool enabled;
  bool suppressDlvRep;
  std::vector<std::string> trace;

  bool respFlag;
  bool aliasInfoFlag;
  bool foundInfoFlag;
  bool routeIdFlag;
  bool srcAddrFlag;
  bool dstAddrFlag;
  bool smeIdFlag;
  bool srcSmeIdFlag;
  bool priorityFlag;
  bool serviceIdFlag;
  bool deliveryModeFlag;
  bool forwardToFlag;
  bool billingFlag;
  bool archivingFlag;
  bool enabledFlag;
  bool suppressDlvRepFlag;
  bool traceFlag;
};

}
}
}
}
#endif
