#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"

#include "SnmpCounter.hpp"

namespace smsc{
namespace snmp{

using namespace xercesc;
using namespace smsc::util::xml;
using namespace smsc::core::buffers;

SnmpCounter* SnmpCounter::instance=0;

const char* cntNames[]={
"accepted",
"rejected",
"retried",
"delivered",
"failed",
"temperror",
"0x014",
"0x058",
"0x00d",
"SDP",
"other"
};

static Hash<int> cntNameToIdx;

struct InitCntNameToIdx{
  InitCntNameToIdx()
  {
    for(int i=0;i<sizeof(cntNames)/sizeof(cntNames[0]);i++)
    {
      cntNameToIdx.Insert(cntNames[i],i);
    }
  }
}cntNameToIdxIniter;


template <int N>
int FindMapping(const XmlStr& name,char* (&names)[N],int (&values)[N],int defValue)
{
  for(int i=0;i<N;i++)
  {
    if(name==names[i])
    {
      return values[i];
    }
  }
  return defValue;
}

static void ParseRecord(DOMNode* record,SnmpCounter::SeverityLimits& lim,SnmpCounter::SmeTrapSeverities& trpsvrt)
{
  for(DOMNode *node = record->getFirstChild(); node != 0; node = node->getNextSibling())
  {
    if (node->getNodeType() != DOMNode::ELEMENT_NODE)
    {
      continue;
    }

    DOMNamedNodeMap *attrs = node->getAttributes();
    if(!attrs)
    {
      throw smsc::util::Exception("Invalid snmp.xml configuration file");
    }

    XmlStr nodeName(node->getNodeName());

    if(nodeName=="counter")
    {
      XmlStr cntName(attrs->getNamedItem(XmlStr("name"))->getNodeValue());
      const int* idxPtr=cntNameToIdx.GetPtr(cntName.c_str());
      if(!idxPtr)
      {
        throw smsc::util::Exception("Unknown counter '%s' in snmp.xml",cntName.c_str());
      }
      XmlStr content(node->getTextContent());
      if(sscanf(content.c_str(),"%d,%d,%d,%d",
                &lim.values[*idxPtr][0],
                &lim.values[*idxPtr][1],
                &lim.values[*idxPtr][2],
                &lim.values[*idxPtr][3])!=4)
      {
        throw smsc::util::Exception("Failed to parse severity limits for counter '%s' in snmp.xml:%s",cntName.c_str(),content.c_str());
      }

    }else if(nodeName=="severity")
    {
      static char* svrtNames[]={
        "off","normal","warning","minor","major","critical"
      };
      static int svrtValues[]={
        -1,
        smsc::snmp::SnmpAgent::NORMAL,
        smsc::snmp::SnmpAgent::WARNING,
        smsc::snmp::SnmpAgent::MINOR,
        smsc::snmp::SnmpAgent::MAJOR,
        smsc::snmp::SnmpAgent::CRITICAL
      };
      XmlStr eventName(attrs->getNamedItem(XmlStr("event"))->getNodeValue());
      XmlStr value(attrs->getNamedItem(XmlStr("value"))->getNodeValue());
      if(eventName=="register")
      {
        trpsvrt.onRegister=FindMapping(value,svrtNames,svrtValues,trpsvrt.onRegister);
      }else
      if(eventName=="unregister")
      {
        trpsvrt.onUnregister=FindMapping(value,svrtNames,svrtValues,trpsvrt.onUnregister);
      }else
      if(eventName=="registerFailed")
      {
        trpsvrt.onRegisterFailed=FindMapping(value,svrtNames,svrtValues,trpsvrt.onRegisterFailed);
      }else
      if(eventName=="unregisterFailed")
      {
        trpsvrt.onUnregisterFailed=FindMapping(value,svrtNames,svrtValues,trpsvrt.onUnregisterFailed);
      }
    }
  }
}

void SnmpCounter::LoadCfg(const char* fileName)
{
  sync::MutexGuard mg(cfgMtx);
  DOMTreeReader reader;
  configFileName=fileName;
  DOMDocument* doc=reader.read(fileName);
  DOMElement *elem = doc->getDocumentElement();
  DOMNodeList *list;
  list = elem->getElementsByTagName(XmlStr("counterInterval"));
  interval=300;
  if(list->getLength()!=0)
  {
    DOMNode* intervalNode=list->item(0);
    XmlStr value(intervalNode->getAttributes()->getNamedItem(XmlStr("value"))->getNodeValue());
    interval=atoi(value.c_str());
  }else
  {
    smsc_log_warn(log,"counterInterval not found, using default(300)");
  }
  list = elem->getElementsByTagName(XmlStr("default"));
  DOMNode *record = list->item(0);
  ParseRecord(record,defSvrtLim,defTrpSvrt);
  {
    DOMNamedNodeMap *attrs = record->getAttributes();
    XmlStr enabled(attrs->getNamedItem(XmlStr("enabled"))->getNodeValue());
    defSvrtLim.enabled=enabled=="true";
  }
  list = elem->getElementsByTagName(XmlStr("object"));
  size_t listLength=list->getLength();
  svrtHash.Empty();
  int cnt=0;
  for(size_t i=0;i<listLength;i++)
  {
    record=list->item(i);
    DOMNamedNodeMap *attrs = record->getAttributes();
    XmlStr objId(attrs->getNamedItem(XmlStr("id"))->getNodeValue());
    XmlStr enabled(attrs->getNamedItem(XmlStr("enabled"))->getNodeValue());
    SeverityLimits lmt;
    lmt=defSvrtLim;
    lmt.enabled=enabled=="true";
    SmeTrapSeverities trpsvrt;
    ParseRecord(record,lmt,trpsvrt);
    svrtHash.Insert(objId.c_str(),lmt);
    smeTrpSvrts.Insert(objId.c_str(),trpsvrt);
    cnt++;
  }
  smsc_log_info(log,"LoadCfg: %d objects loaded",cnt);
}

void SnmpCounter::Check(const char* name,Counters& cnt,Severities& svrt)
{
  static const char* alarmIdName[cntCountersNumber]=
  {
    "Accepted",
    "Rejected",
    "Retried",
    "Delivered",
    "Failed",
    "TempError",
    "0x014",
    "0x058",
    "0x00d",
    "SDP",
    "Other",
  };
  char msg[1024];
  char alarmId[64];
  using smsc::snmp::SnmpAgent;
  for(int i=0;i<cntCountersNumber;i++)
  {
    SeverityType curSvrt=GetSeverityValue(name,i,cnt.values[i]);
    if(svrt.values[i]!=curSvrt)
    {
      sprintf(alarmId,"%s_%s",name,alarmIdName[i]);
      sprintf(msg,"%s SMPP %s Threshold crossed (AlarmId=%s; severity=%d)",curSvrt==SnmpAgent::NORMAL?"CLEARED":"ACTIVE",alarmId,alarmId,curSvrt);
      smsc_log_info(log,"SNMPTRAP:%s",msg);
      smsc::snmp::SnmpAgent::trap(alarmId,"SMPP",curSvrt,msg);
      svrt.values[i]=curSvrt;
    }
    cnt.values[i]=0;
  }
}


} //system
} //smsc
