#include "FraudControl.hpp"
#include "util/debug.h"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"
#include <xercesc/dom/DOM.hpp>
#include "util/debug.h"

namespace smsc{
namespace system{
namespace mapio{


FraudControl* FraudControl::instance=0;

void FraudControl::Init(const char* argFileName)
{
  if(!instance)
  {
    instance=new FraudControl;
  }else
  {
    __warning__("Duplicate FraudConrol init");
  }
  instance->whiteList=new buf::Hash<int>;
  instance->fileName=argFileName;
  instance->Reload();
}

void FraudControl::Shutdown()
{
  delete instance;
  instance=0;
}


void FraudControl::Reload()
{
  using namespace xercesc;
  using namespace smsc::util::xml;
  buf::Hash<int> *h=new buf::Hash<int>;
  smsc::util::xml::DOMTreeReader reader;
  DOMDocument *document = reader.read(fileName.c_str());
  DOMElement *elem = document->getDocumentElement();

  DOMNodeList *policy = elem->getElementsByTagName(XmlStr("policy"));
  if(policy->getLength()!=0)
  {
    DOMNode *policyNode=policy->item(0);
    DOMNodeList *tail=policyNode->getChildNodes();
    for(int i=0;i<tail->getLength();i++)
    {
      if(tail->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
      {
        DOMNode *tailNode=tail->item(i);
        XmlStr name(tailNode->getNodeName());
        XmlStr value(tailNode->getAttributes()->getNamedItem(XmlStr("value"))->getNodeValue());
        if(name=="tail")
        {
          ignoreTail=atoi(value.c_str());
        }else if(name=="enableCheck")
        {
          enabledCheck=value=="true";
        }else if(name=="enableReject")
        {
          enableReject=value=="true";
        }
      }
    }
  }
  DOMNodeList *wl=elem->getElementsByTagName(XmlStr("whitelist"));
  if(wl->getLength()==0)
  {
    whiteList=h;
    return;
  }
  DOMNode *wlnode=wl->item(0);
  DOMNodeList *mscs=wlnode->getChildNodes();
  for(int i=0;i<mscs->getLength();i++)
  {
    DOMNode* n=mscs->item(i);
    if(n->getNodeType() == DOMNode::ELEMENT_NODE)
    {
      XmlStr value(n->getAttributes()->getNamedItem(XmlStr("value"))->getNodeValue());
      __trace2__("fraud:white list %s",value.c_str());
      h->Insert(value.c_str(),1);
    }
  }
  whiteList=h;
}

}
}
}

