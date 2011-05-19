#include "NetworkProfiles.hpp"
#include "util/Exception.hpp"
#include "util/xml/utilFunctions.h"
#include "util/xml/DOMTreeReader.h"

namespace smsc{
namespace mapio{

NetworkProfiles* NetworkProfiles::instance=0;

void NetworkProfiles::init(const char* configFileName)
{
  if(instance!=0)
  {
    throw smsc::util::Exception("Duplicate Networks init");
  }
  instance=new NetworkProfiles;
  instance->log=smsc::logger::Logger::getInstance("netprof");
  instance->configFile=configFileName;
  instance->reload();
}

void NetworkProfiles::shutdown()
{
  delete instance;
  instance=0;
}

static AbonentStatusMethod parseAbonentStatusMethod(const smsc::util::xml::XmlStr& val)
{
  AbonentStatusMethod rv;
  if(val=="SRI4SM")
  {
    rv=asmSRI4SM;
  }else if(val=="ATI")
  {
    rv=asmATI;
  }else
  {
    throw smsc::util::Exception("Invalid value of 'abonentStatusMethod'='%s'",val.c_str());
  }
  return rv;
}

static void parseParam(NetworkProfile* np,const smsc::util::xml::XmlStr& nm,const smsc::util::xml::XmlStr& val)
{
  if(nm=="ussdOpenDestRef")
  {
    if(val=="destAddr")
    {
      np->drv=drvDestAddr;
    }else if(val=="destIMSI")
    {
      np->drv=drvDestIMSI;
    }else if(val=="empty")
    {
      np->drv=drvEmpty;
    }else
    {
      throw smsc::util::Exception("Invalid value of 'ussdOpenDestRef'='%s'",val.c_str());
    }
  }else if(nm=="abonentStatusMethod")
  {
    np->asMethod=parseAbonentStatusMethod(val);
  }else if(nm=="vlrDetectionMethod")
  {
    np->niVlrMethod=parseAbonentStatusMethod(val);
  }
}

void NetworkProfiles::Clear()
{
  profiles.Clear();
  for(NetworkList::iterator it=netList.begin(),end=netList.end();it!=end;++it)
  {
    delete *it;
  }
}

void NetworkProfiles::reload()
{
  smsc_log_info(log,"Loading network profiles");
  using namespace xercesc;
  using namespace smsc::util::xml;
  smsc::core::synchronization::MutexGuard mg(mtx);
  Clear();
  smsc::util::xml::DOMTreeReader reader;
  DOMDocument *document = reader.read(configFile.c_str());
  if(!document)
  {
    throw smsc::util::Exception("Failed to read network profiles config %s",configFile.c_str());
  }
  DOMElement *elem = document->getDocumentElement();

  int count=0;

  XmlStr nameStr("name");
  XmlStr valStr("value");

  DOMNodeList *nwList= elem->getElementsByTagName(XmlStr("network"));
  for(XMLSize_t i=0;i<nwList->getLength();i++)
  {
    DOMNode* nw=nwList->item(i);
    if(nw->getNodeType()!=DOMNode::ELEMENT_NODE)continue;
    count++;
    NetworkProfile* np=new NetworkProfile;
    np->name=XmlStr(nw->getAttributes()->getNamedItem(nameStr)->getNodeValue()).c_str();
    netList.push_back(np);
    smsc_log_debug(log,"loading profile for network '%s'",np->name.c_str());
    DOMNodeList* mskList;
    XMLSize_t j=0;
    while(nw->getChildNodes()->item(j)->getNodeType()!=DOMNode::ELEMENT_NODE)j++;
    mskList=nw->getChildNodes()->item(j)->getChildNodes();
    for(XMLSize_t k=0;k<mskList->getLength();k++)
    {
      DOMNode* msk=mskList->item(k);
      if(msk->getNodeType()!=DOMNode::ELEMENT_NODE)continue;
      DOMNamedNodeMap* attr=msk->getAttributes();
      XmlStr val(attr->getNamedItem(valStr)->getNodeValue());
      if(NetworkProfile** pptr=profiles.Find(val.c_str()))
      {
        smsc_log_warn(log,"Duplicate mask %s in network %s (previous in %s)",val.c_str(),np->name.c_str(),(*pptr)->name.c_str());
        continue;
      }
      smsc_log_debug(log,"inserting mask %s",val.c_str());
      profiles.Insert(val.c_str(),np);
    }
    j++;
    while(nw->getChildNodes()->item(j)->getNodeType()!=DOMNode::ELEMENT_NODE)j++;
    DOMNodeList* paramList=nw->getChildNodes()->item(j)->getChildNodes();
    for(XMLSize_t k=0;k<paramList->getLength();k++)
    {
      DOMNode* param=paramList->item(k);
      if(param->getNodeType()!=DOMNode::ELEMENT_NODE)continue;
      DOMNamedNodeMap* attr=param->getAttributes();
      XmlStr nm(attr->getNamedItem(nameStr)->getNodeValue());
      XmlStr val(attr->getNamedItem(valStr)->getNodeValue());
      smsc_log_debug(log,"parsing param '%s'='%s'",nm.c_str(),val.c_str());
      parseParam(np,nm,val);
    }
  }
  smsc_log_debug(log,"parsing default params");
  DOMNodeList* def=elem->getElementsByTagName(XmlStr("default"));
  XMLSize_t j=0;
  while(def->item(j)->getNodeType()!=DOMNode::ELEMENT_NODE)j++;
  DOMNodeList* paramList=def->item(j)->getChildNodes();
  j=0;
  while(paramList->item(j)->getNodeType()!=DOMNode::ELEMENT_NODE)j++;
  paramList=paramList->item(j)->getChildNodes();
  for(XMLSize_t k=0;k<paramList->getLength();k++)
  {
    DOMNode* param=paramList->item(k);
    if(param->getNodeType()!=DOMNode::ELEMENT_NODE)continue;
    DOMNamedNodeMap* attr=param->getAttributes();
    XmlStr nm(attr->getNamedItem(nameStr)->getNodeValue());
    XmlStr val(attr->getNamedItem(valStr)->getNodeValue());
    smsc_log_debug(log,"parsing param '%s'='%s'",nm.c_str(),val.c_str());
    parseParam(&defaultProfile,nm,val);
  }
  smsc_log_info(log,"Loaded %d profiles",count);
}

const NetworkProfile& NetworkProfiles::lookup(const smsc::sms::Address& addr)
{
  smsc::core::synchronization::MutexGuard mg(mtx);
  NetworkProfile** pptr=profiles.Find(addr.value);
  if(pptr)
  {
    return **pptr;
  }else
  {
    return defaultProfile;
  }
}

}
}
