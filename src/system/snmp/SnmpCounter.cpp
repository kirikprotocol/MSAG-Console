#include "SnmpCounter.hpp"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"

namespace smsc{
namespace system{

using namespace xercesc;
using namespace smsc::util::xml;

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


static void ParseRecord(DOMNode* record,SnmpCounter::SeverityLimits& lim)
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
  ParseRecord(record,defSvrtLim);
  list = elem->getElementsByTagName(XmlStr("object"));
  int listLength=list->getLength();
  svrtHash.Empty();
  int cnt=0;
  for(int i=0;i<listLength;i++)
  {
    record=list->item(i);
    DOMNamedNodeMap *attrs = record->getAttributes();
    XmlStr objId(attrs->getNamedItem(XmlStr("id"))->getNodeValue());
    XmlStr enabled(attrs->getNamedItem(XmlStr("enabled"))->getNodeValue());
    SeverityLimits lmt;
    lmt=defSvrtLim;
    lmt.enabled=enabled=="true";
    ParseRecord(record,lmt);
    svrtHash.Insert(objId.c_str(),lmt);
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
