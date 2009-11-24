#ifndef __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_TRACEROUTERESP_HPP__
#define __GENERATED_MESSAGE_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_TRACEROUTERESP_HPP__

#include <inttypes.h>
#include <string>
#include <vector>
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "Response.hpp"


#ident "@(#) TraceRouteResp version 1.0"



namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

typedef std::vector<std::string> string_list;

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
      rv+=foundInfo;
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
      sprintf(buf,"%u",(unsigned int)priority);
      rv+=buf;
    }
    if(serviceIdFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="serviceId=";
      sprintf(buf,"%u",(unsigned int)serviceId);
      rv+=buf;
    }
    if(deliveryModeFlag)
    {
      if(rv.length()>0)
      {
        rv+=";";
      }
      rv+="deliveryMode=";
      sprintf(buf,"%u",(unsigned int)deliveryMode);
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
      rv+=billing?"true":"false";
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
      for(string_list::const_iterator it=trace.begin(),end=trace.end();it!=end;it++)
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
  const std::string& getAliasInfo()const
  {
    if(!aliasInfoFlag)
    {
      throw protogen::framework::FieldIsNullException("aliasInfo");
    }
    return aliasInfo;
  }
  void setAliasInfo(const std::string& value)
  {
    aliasInfo=value;
    aliasInfoFlag=true;
  }
  bool hasAliasInfo()const
  {
    return aliasInfoFlag;
  }
  const std::string& getFoundInfo()const
  {
    if(!foundInfoFlag)
    {
      throw protogen::framework::FieldIsNullException("foundInfo");
    }
    return foundInfo;
  }
  void setFoundInfo(const std::string& value)
  {
    foundInfo=value;
    foundInfoFlag=true;
  }
  bool hasFoundInfo()const
  {
    return foundInfoFlag;
  }
  const std::string& getRouteId()const
  {
    if(!routeIdFlag)
    {
      throw protogen::framework::FieldIsNullException("routeId");
    }
    return routeId;
  }
  void setRouteId(const std::string& value)
  {
    routeId=value;
    routeIdFlag=true;
  }
  bool hasRouteId()const
  {
    return routeIdFlag;
  }
  const std::string& getSrcAddr()const
  {
    if(!srcAddrFlag)
    {
      throw protogen::framework::FieldIsNullException("srcAddr");
    }
    return srcAddr;
  }
  void setSrcAddr(const std::string& value)
  {
    srcAddr=value;
    srcAddrFlag=true;
  }
  bool hasSrcAddr()const
  {
    return srcAddrFlag;
  }
  const std::string& getDstAddr()const
  {
    if(!dstAddrFlag)
    {
      throw protogen::framework::FieldIsNullException("dstAddr");
    }
    return dstAddr;
  }
  void setDstAddr(const std::string& value)
  {
    dstAddr=value;
    dstAddrFlag=true;
  }
  bool hasDstAddr()const
  {
    return dstAddrFlag;
  }
  const std::string& getSmeId()const
  {
    if(!smeIdFlag)
    {
      throw protogen::framework::FieldIsNullException("smeId");
    }
    return smeId;
  }
  void setSmeId(const std::string& value)
  {
    smeId=value;
    smeIdFlag=true;
  }
  bool hasSmeId()const
  {
    return smeIdFlag;
  }
  const std::string& getSrcSmeId()const
  {
    if(!srcSmeIdFlag)
    {
      throw protogen::framework::FieldIsNullException("srcSmeId");
    }
    return srcSmeId;
  }
  void setSrcSmeId(const std::string& value)
  {
    srcSmeId=value;
    srcSmeIdFlag=true;
  }
  bool hasSrcSmeId()const
  {
    return srcSmeIdFlag;
  }
  uint32_t getPriority()const
  {
    if(!priorityFlag)
    {
      throw protogen::framework::FieldIsNullException("priority");
    }
    return priority;
  }
  void setPriority(uint32_t value)
  {
    priority=value;
    priorityFlag=true;
  }
  bool hasPriority()const
  {
    return priorityFlag;
  }
  uint32_t getServiceId()const
  {
    if(!serviceIdFlag)
    {
      throw protogen::framework::FieldIsNullException("serviceId");
    }
    return serviceId;
  }
  void setServiceId(uint32_t value)
  {
    serviceId=value;
    serviceIdFlag=true;
  }
  bool hasServiceId()const
  {
    return serviceIdFlag;
  }
  uint32_t getDeliveryMode()const
  {
    if(!deliveryModeFlag)
    {
      throw protogen::framework::FieldIsNullException("deliveryMode");
    }
    return deliveryMode;
  }
  void setDeliveryMode(uint32_t value)
  {
    deliveryMode=value;
    deliveryModeFlag=true;
  }
  bool hasDeliveryMode()const
  {
    return deliveryModeFlag;
  }
  const std::string& getForwardTo()const
  {
    if(!forwardToFlag)
    {
      throw protogen::framework::FieldIsNullException("forwardTo");
    }
    return forwardTo;
  }
  void setForwardTo(const std::string& value)
  {
    forwardTo=value;
    forwardToFlag=true;
  }
  bool hasForwardTo()const
  {
    return forwardToFlag;
  }
  bool getBilling()const
  {
    if(!billingFlag)
    {
      throw protogen::framework::FieldIsNullException("billing");
    }
    return billing;
  }
  void setBilling(bool value)
  {
    billing=value;
    billingFlag=true;
  }
  bool hasBilling()const
  {
    return billingFlag;
  }
  bool getArchiving()const
  {
    if(!archivingFlag)
    {
      throw protogen::framework::FieldIsNullException("archiving");
    }
    return archiving;
  }
  void setArchiving(bool value)
  {
    archiving=value;
    archivingFlag=true;
  }
  bool hasArchiving()const
  {
    return archivingFlag;
  }
  bool getEnabled()const
  {
    if(!enabledFlag)
    {
      throw protogen::framework::FieldIsNullException("enabled");
    }
    return enabled;
  }
  void setEnabled(bool value)
  {
    enabled=value;
    enabledFlag=true;
  }
  bool hasEnabled()const
  {
    return enabledFlag;
  }
  bool getSuppressDlvRep()const
  {
    if(!suppressDlvRepFlag)
    {
      throw protogen::framework::FieldIsNullException("suppressDlvRep");
    }
    return suppressDlvRep;
  }
  void setSuppressDlvRep(bool value)
  {
    suppressDlvRep=value;
    suppressDlvRepFlag=true;
  }
  bool hasSuppressDlvRep()const
  {
    return suppressDlvRepFlag;
  }
  const string_list& getTrace()const
  {
    if(!traceFlag)
    {
      throw protogen::framework::FieldIsNullException("trace");
    }
    return trace;
  }
  void setTrace(const string_list& value)
  {
    trace=value;
    traceFlag=true;
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
      throw protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!aliasInfoFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("aliasInfo");
    }
    if(!foundInfoFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("foundInfo");
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
    ds.writeStrLV(foundInfo);
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
    ds.writeInt32LV(deliveryMode);
    }
    if(forwardToFlag)
    {
      ds.writeTag(forwardToTag);
    ds.writeStrLV(forwardTo);
    }
    if(billingFlag)
    {
      ds.writeTag(billingTag);
    ds.writeBoolLV(billing);
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
    ds.writeStrLstLV(trace);
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
    //  throw protogen::framework::IncompatibleVersionException("TraceRouteResp");
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
        case aliasInfoTag:
        {
          if(aliasInfoFlag)
          {
            throw protogen::framework::DuplicateFieldException("aliasInfo");
          }
          aliasInfo=ds.readStrLV();
          aliasInfoFlag=true;
        }break;
        case foundInfoTag:
        {
          if(foundInfoFlag)
          {
            throw protogen::framework::DuplicateFieldException("foundInfo");
          }
          foundInfo=ds.readStrLV();
          foundInfoFlag=true;
        }break;
        case routeIdTag:
        {
          if(routeIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("routeId");
          }
          routeId=ds.readStrLV();
          routeIdFlag=true;
        }break;
        case srcAddrTag:
        {
          if(srcAddrFlag)
          {
            throw protogen::framework::DuplicateFieldException("srcAddr");
          }
          srcAddr=ds.readStrLV();
          srcAddrFlag=true;
        }break;
        case dstAddrTag:
        {
          if(dstAddrFlag)
          {
            throw protogen::framework::DuplicateFieldException("dstAddr");
          }
          dstAddr=ds.readStrLV();
          dstAddrFlag=true;
        }break;
        case smeIdTag:
        {
          if(smeIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("smeId");
          }
          smeId=ds.readStrLV();
          smeIdFlag=true;
        }break;
        case srcSmeIdTag:
        {
          if(srcSmeIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("srcSmeId");
          }
          srcSmeId=ds.readStrLV();
          srcSmeIdFlag=true;
        }break;
        case priorityTag:
        {
          if(priorityFlag)
          {
            throw protogen::framework::DuplicateFieldException("priority");
          }
          priority=ds.readInt32LV();
          priorityFlag=true;
        }break;
        case serviceIdTag:
        {
          if(serviceIdFlag)
          {
            throw protogen::framework::DuplicateFieldException("serviceId");
          }
          serviceId=ds.readInt32LV();
          serviceIdFlag=true;
        }break;
        case deliveryModeTag:
        {
          if(deliveryModeFlag)
          {
            throw protogen::framework::DuplicateFieldException("deliveryMode");
          }
          deliveryMode=ds.readInt32LV();
          deliveryModeFlag=true;
        }break;
        case forwardToTag:
        {
          if(forwardToFlag)
          {
            throw protogen::framework::DuplicateFieldException("forwardTo");
          }
          forwardTo=ds.readStrLV();
          forwardToFlag=true;
        }break;
        case billingTag:
        {
          if(billingFlag)
          {
            throw protogen::framework::DuplicateFieldException("billing");
          }
          billing=ds.readBoolLV();
          billingFlag=true;
        }break;
        case archivingTag:
        {
          if(archivingFlag)
          {
            throw protogen::framework::DuplicateFieldException("archiving");
          }
          archiving=ds.readBoolLV();
          archivingFlag=true;
        }break;
        case enabledTag:
        {
          if(enabledFlag)
          {
            throw protogen::framework::DuplicateFieldException("enabled");
          }
          enabled=ds.readBoolLV();
          enabledFlag=true;
        }break;
        case suppressDlvRepTag:
        {
          if(suppressDlvRepFlag)
          {
            throw protogen::framework::DuplicateFieldException("suppressDlvRep");
          }
          suppressDlvRep=ds.readBoolLV();
          suppressDlvRepFlag=true;
        }break;
        case traceTag:
        {
          if(traceFlag)
          {
            throw protogen::framework::DuplicateFieldException("trace");
          }
          ds.readStrLstLV(trace);
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
      throw protogen::framework::MandatoryFieldMissingException("resp");
    }
    if(!aliasInfoFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("aliasInfo");
    }
    if(!foundInfoFlag)
    {
      throw protogen::framework::MandatoryFieldMissingException("foundInfo");
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
  static const uint32_t aliasInfoTag=2;
  static const uint32_t foundInfoTag=3;
  static const uint32_t routeIdTag=4;
  static const uint32_t srcAddrTag=5;
  static const uint32_t dstAddrTag=6;
  static const uint32_t smeIdTag=7;
  static const uint32_t srcSmeIdTag=8;
  static const uint32_t priorityTag=9;
  static const uint32_t serviceIdTag=10;
  static const uint32_t deliveryModeTag=11;
  static const uint32_t forwardToTag=12;
  static const uint32_t billingTag=13;
  static const uint32_t archivingTag=14;
  static const uint32_t enabledTag=15;
  static const uint32_t suppressDlvRepTag=16;
  static const uint32_t traceTag=17;

  uint32_t seqNum;

  Response resp;
  std::string aliasInfo;
  std::string foundInfo;
  std::string routeId;
  std::string srcAddr;
  std::string dstAddr;
  std::string smeId;
  std::string srcSmeId;
  uint32_t priority;
  uint32_t serviceId;
  uint32_t deliveryMode;
  std::string forwardTo;
  bool billing;
  bool archiving;
  bool enabled;
  bool suppressDlvRep;
  string_list trace;

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
}
#endif
