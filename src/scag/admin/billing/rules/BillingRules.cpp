#include "BillingRules.hpp"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/utilFunctions.h"
#include <memory>


namespace smsc{
namespace scag{
namespace billing{
namespace rules{

using namespace smsc::util::xml;

BillingRulesManager* BillingRulesManager::instancePtr=0;
Mutex BillingRulesManager::instanceMtx;

void BillingRulesManager::Init(const char* fileName)
{
  instancePtr=new BillingRulesManager;
  instancePtr->Ref();
  instancePtr->Load(fileName);
}

void BillingRulesManager::Reload(const char* fileName)
{
  BillingRulesManager *old=0;
  {
    MutexGuard mg(instanceMtx);
    {
      MutexGuard g(instancePtr->refMtx);
      if(--instancePtr->refCount==0)
      {
        old=instancePtr;
      }
    }
  }

  BillingRulesManager* newPtr=new BillingRulesManager;
  newPtr->Ref();
  newPtr->Load(fileName);
  {
    MutexGuard mg(instanceMtx);
    instancePtr=newPtr;
  }


  if(old)delete old;
}

void BillingRulesManager::Uninit()
{
  MutexGuard mg(instanceMtx);
  instancePtr->Unref();
}

class NodeBuilder{
public:
  static void BuildNode(BillingRule* rule,DOMNode* node)
  {
    BuildRoot(rule->rootNode,node);
  }

  static void BuildRoot(BillingRule::Node*& rootNode,DOMNode* node)
  {
    BillingRule::Node* lastNode=0;

    DOMNodeList *childs = node->getChildNodes();
    unsigned childsLength = childs->getLength();
    for (unsigned j=0; j<childsLength; j++)
    {
      DOMNode *child = childs->item(j);
      if (child->getNodeType() == DOMNode::ELEMENT_NODE)
      {
        XmlStr nodeName(child->getNodeName());
        std::auto_ptr<BillingRule::Node> newNode(new BillingRule::Node);
        if(nodeName=="if")
        {
          BuildIf(newNode.get(),child);
        }else if(nodeName=="bill")
        {
           newNode.get()->nodeType=BillingRule::Node::ntBill;
           if(child->getAttributes()->getNamedItem(XmlStr("serviceId")))
           {
             XmlStr serviceId(child->getAttributes()->getNamedItem(XmlStr("serviceId"))->getNodeValue());
             newNode.get()->billServiceId=atoi(serviceId.c_str());
           }else
           {
             newNode.get()->billServiceId=-1;
           }
        }else if(nodeName=="nobill")
        {
           newNode.get()->nodeType=BillingRule::Node::ntNobill;
        }else
        {
          throw Exception("Node %s not expected in root!",nodeName.c_str());
        }

        if(lastNode)
        {
          lastNode->nextNode=newNode.release();
          lastNode=lastNode->nextNode;
        }else
        {
          rootNode=newNode.release();
          lastNode=rootNode;
        }
      }
    }
  }

  static void BuildField(BillingRule::Node* fieldNode,BillingRule::Node::NodeType nodeType,DOMNode* node)
  {
    fieldNode->nodeType=nodeType;
    struct MTData{
      const char* mtName;
      BillingRule::Node::MatchTarget mt;
    };
    MTData addMtData[]={
      {"oa",BillingRule::Node::mtOa},
      {"da",BillingRule::Node::mtDa}
    };
    MTData smeMtData[]={
      {"src",BillingRule::Node::mtSrc},
      {"dst",BillingRule::Node::mtDst}
    };
    MTData routeMtData[]={
      {"id",BillingRule::Node::mtId},
      {"providerId",BillingRule::Node::mtProviderId},
      {"serviceId",BillingRule::Node::mtServiceId}
    };

    MTData *mtData=0;
    int mtDataLen;
#define NB_SET_MT_DATA(arr) mtData=arr;mtDataLen=sizeof(arr)/sizeof(arr[0])
    switch(nodeType)
    {
      case BillingRule::Node::ntAddr:NB_SET_MT_DATA(addMtData);break;
      case BillingRule::Node::ntSme:NB_SET_MT_DATA(smeMtData);break;
      case BillingRule::Node::ntRoute:NB_SET_MT_DATA(routeMtData);break;
      default: throw Exception("Unknown field node type %d",nodeType);
    }
#undef NB_SET_MT_DATA
    if(!node->getAttributes()->getNamedItem(XmlStr("what")))
    {
      throw Exception("what attribute required for field node!");
    }
    XmlStr what(node->getAttributes()->getNamedItem(XmlStr("what"))->getNodeValue());

    bool found=false;
    for(int i=0;i<mtDataLen;i++)
    {
      if(what==mtData[i].mtName)
      {
        found=true;
        fieldNode->matchTarget=mtData[i].mt;
        break;
      }
    }
    if(!found)
    {
      throw Exception("Unknown what value for field node: %s",what.c_str());
    }
    if(node->getAttributes()->getNamedItem(XmlStr("equal")) &&
       node->getAttributes()->getNamedItem(XmlStr("match")))
    {
      throw Exception("Both equal and match found, only one expected in field node");
    }
    if(!node->getAttributes()->getNamedItem(XmlStr("equal")) &&
       !node->getAttributes()->getNamedItem(XmlStr("match")))
    {
      throw Exception("One of equal or match must be present in field node");
    }
    if(node->getAttributes()->getNamedItem(XmlStr("equal")))
    {
      XmlStr eq(node->getAttributes()->getNamedItem(XmlStr("equal"))->getNodeValue());
      fieldNode->matchString=new std::string;
      fieldNode->matchType=BillingRule::Node::mtEqual;
      *fieldNode->matchString=eq;
    }else
    {
      XmlStr rx(node->getAttributes()->getNamedItem(XmlStr("match"))->getNodeValue());
      fieldNode->matchRegExp=new smsc::util::regexp::RegExp;
      fieldNode->matchType=BillingRule::Node::mtRegExp;
      if(!fieldNode->matchRegExp->Compile(rx))
      {
        throw Exception("Invalid regexp:%s",rx.c_str());
      }
    }
  }


  static void BuildIf(BillingRule::Node* ifNode,DOMNode* node)
  {
    BillingRule::Node* lastNode=0;

    ifNode->nodeType=BillingRule::Node::ntIf;

    DOMNodeList *childs = node->getChildNodes();
    unsigned childsLength = childs->getLength();
    for (unsigned j=0; j<childsLength; j++)
    {
      DOMNode *child = childs->item(j);
      if (child->getNodeType() == DOMNode::ELEMENT_NODE)
      {
        XmlStr nodeName(child->getNodeName());
        std::auto_ptr<BillingRule::Node> newNode(new BillingRule::Node);
        if(      nodeName=="and"  )
        {
          BuildOp(newNode.get(),BillingRule::Node::ntAnd,child);
        }else if(nodeName=="or"   )
        {
          BuildOp(newNode.get(),BillingRule::Node::ntOr,child);
        }else if(nodeName=="addr" )
        {
          BuildField(newNode.get(),BillingRule::Node::ntAddr,child);
        }else if(nodeName=="sme"  )
        {
          BuildField(newNode.get(),BillingRule::Node::ntSme,child);
        }else if(nodeName=="route")
        {
          BuildField(newNode.get(),BillingRule::Node::ntRoute,child);
        }else if(nodeName=="then" )
        {
          if(ifNode->thenNode)
          {
            throw Exception("Duplicate <then> node found!");
          }
          BuildRoot(ifNode->thenNode,child);
          continue;
        }else if(nodeName=="else" )
        {
          if(!ifNode->thenNode)
          {
            throw Exception("<else> node without <than> found!");
          }
          if(ifNode->elseNode)
          {
            throw Exception("Duplicate <else> node found!");
          }
          BuildRoot(ifNode->elseNode,child);
          continue;
        }else
        {
          throw Exception("Node %s not expected in if!",nodeName.c_str());
        }

        if(ifNode->thenNode)
        {
          throw Exception("Node %s not expected after then",nodeName.c_str());
        }

        if(lastNode)
        {
          lastNode->nextNode=newNode.release();
          lastNode=lastNode->nextNode;
        }else
        {
          ifNode->childNode=newNode.release();
          lastNode=ifNode->childNode;
        }
      }
    }
    if(!ifNode->childNode)
    {
      throw Exception("If node cannot be empty");
    }
  }

  static void BuildOp(BillingRule::Node* opNode,BillingRule::Node::NodeType nodeType,DOMNode* node)
  {
    BillingRule::Node* lastNode=0;

    opNode->nodeType=nodeType;

    DOMNodeList *childs = node->getChildNodes();
    unsigned childsLength = childs->getLength();
    for (unsigned j=0; j<childsLength; j++)
    {
      DOMNode *child = childs->item(j);
      if (child->getNodeType() == DOMNode::ELEMENT_NODE)
      {
        XmlStr nodeName(child->getNodeName());
        std::auto_ptr<BillingRule::Node> newNode(new BillingRule::Node);
        if(      nodeName=="and"  )
        {
          BuildOp(newNode.get(),BillingRule::Node::ntAnd,child);
        }else if(nodeName=="or"   )
        {
          BuildOp(newNode.get(),BillingRule::Node::ntOr,child);
        }else if(nodeName=="addr" )
        {
          BuildField(newNode.get(),BillingRule::Node::ntAddr,child);
        }else if(nodeName=="sme"  )
        {
          BuildField(newNode.get(),BillingRule::Node::ntSme,child);
        }else if(nodeName=="route")
        {
          BuildField(newNode.get(),BillingRule::Node::ntRoute,child);
        }else
        {
          throw Exception("Unexpected node:%s",nodeName.c_str());
        }

        if(lastNode)
        {
          lastNode->nextNode=newNode.release();
          lastNode=lastNode->nextNode;
        }else
        {
          opNode->childNode=newNode.release();
          lastNode=opNode->childNode;
        }
      }
    }
  }

};

void BillingRulesManager::Load(const char* fileName)
{
  DOMTreeReader reader;
  DOMDocument *document = reader.read(fileName);
  DOMElement *elem = document->getDocumentElement();
  DOMNodeList *list = elem->getElementsByTagName(XmlStr("rule"));
  unsigned listLength = list->getLength();
  for (unsigned i=0; i<listLength; i++)
  {
    DOMNode *node = list->item(i);
    XmlStr name(node->getAttributes()->getNamedItem(XmlStr("name"))->getNodeValue());
    std::auto_ptr<BillingRule> rule(new BillingRule);
    NodeBuilder::BuildNode(rule.get(),node);
    rules.Insert(name,rule.release());
  }

}



}//namespace rules
}//namespace billing
}//namespace scag
}//namespace smsc
