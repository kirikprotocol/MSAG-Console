#ifndef __SMSC_SMPPGW_BILLING_RULES_BILLINGRULES_HPP__
#define __SMSC_SMPPGW_BILLING_RULES_BILLINGRULES_HPP__

#include "core/synchronization/Mutex.hpp"
#include "sms/sms.h"
#include "core/buffers/Hash.hpp"
#include <string>
#include "util/Exception.hpp"
#include "util/regexp/RegExp.hpp"

namespace smsc{
namespace smppgw{
namespace billing{
namespace rules{

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::sms::SMS;
using smsc::util::Exception;

class NodeBuilder;

class BillingRule{
public:
  BillingRule():rootNode(0)
  {
  }

  ~BillingRule()
  {
    if(rootNode)delete rootNode;
  }


  bool queryBill(SMS& sms)const
  {
    return rootNode->Match(sms)==Node::mrBilled;
  }

protected:
  BillingRule(const BillingRule&);
  BillingRule& operator=(const BillingRule&);

  struct Node{

    enum{MAX_WHAT_LENGTH=32};

    enum NodeType{
      ntIf,ntAddr,ntSme,ntRoute,ntOr,ntAnd,ntBill,ntNobill,ntInvalid
    };

    enum MatchType{
      mtEqual,mtRegExp
    };

    enum MatchTarget{
      mtOa,mtDa,mtDDa,mtSrc,mtDst,mtId,mtProviderId,mtServiceId
    };

    NodeType nodeType;
    MatchTarget matchTarget;
    MatchType matchType;

    union{
      std::string* matchString;
      int billServiceId;
      smsc::util::regexp::RegExp* matchRegExp;
    };

    Node* childNode;
    Node* nextNode;
    Node* thenNode;
    Node* elseNode;

    Node():nodeType(ntInvalid),childNode(0),nextNode(0),thenNode(0),elseNode(0),matchString(0)
    {
    }

    ~Node()
    {
      switch(nodeType)
      {
        case ntAddr:
        case ntSme:
        case ntRoute:
        {
          if(matchType==mtEqual)
          {
            if(matchString)delete matchString;
          }else
          {
            if(matchRegExp)delete matchRegExp;
          }
        }break;
        case ntIf:
          if(thenNode)delete thenNode;
          if(elseNode)delete elseNode;
          if(nextNode)delete nextNode;
          //no break by design
        case ntOr:
        case ntAnd:
        {
          Node* n=childNode;
          while(n)
          {
            Node* q=n->nextNode;
            delete n;
            n=q;
          }
        }break;
        case ntBill:
        case ntNobill:break;
      }
      nodeType=ntInvalid;
      childNode=0;
      nextNode=0;
      thenNode=0;
      elseNode=0;

    }

    bool MatchField(const char* fld)const
    {
      if(matchType==mtEqual)
      {
        __trace2__("FieldEqual '%s'<=>'%s'",fld,matchString?matchString->c_str():"");
        return matchString && (*matchString==fld);
      }else if(matchType==mtRegExp)
      {
        smsc::util::regexp::SMatch sm[99];
        int n=99;
        bool res=matchRegExp->Match(fld,sm,n)!=0;
        __trace2__("FieldRegExp:%s,res=%s",fld,res?"true":"false");
        return res;
      }
      __warning2__("Unknown match type %d",matchType);
      return false;
    }

    enum MatchResult{
      mrNotMatched,mrMatched,mrBilled,mrNotBilled
    };
    MatchResult Match(SMS& sms)const
    {
      switch(nodeType)
      {
        case ntIf:
        {
          __trace__("if");
          MatchResult mr=mrNotMatched;
          if(childNode->Match(sms)==mrMatched)
          {
            mr=thenNode->Match(sms);
            __trace2__("then:%d",mr);
          }else
          {
            if(elseNode)
            {
              mr=elseNode->Match(sms);
            __trace2__("else:%d",mr);
            }
          }
          if(mr!=mrNotMatched)
          {
            return mr;
          }
          if(nextNode)
          {
            return nextNode->Match(sms);
          }
          return mr;
        }break;
        case ntAddr:
        {
          switch(matchTarget)
          {
            case mtOa:
            {
              char buf[smsc::sms::MAX_FULL_ADDRESS_VALUE_LENGTH];
              sms.originatingAddress.toString(buf,sizeof(buf));
              return MatchField(buf)?mrMatched:mrNotMatched;
            }
            case mtDa:
            {
              char buf[smsc::sms::MAX_FULL_ADDRESS_VALUE_LENGTH];
              sms.destinationAddress.toString(buf,sizeof(buf));
              return MatchField(buf)?mrMatched:mrNotMatched;
            }
            default:throw Exception("ntAddr - unknown matchTarget %d",matchTarget);
          }
        }break;
        case ntSme:
        {
          switch(matchTarget)
          {
            case mtSrc:return MatchField(sms.srcSmeId)?mrMatched:mrNotMatched;
            case mtDst:return MatchField(sms.dstSmeId)?mrMatched:mrNotMatched;
            default:throw Exception("ntSme - unknown matchTarget %d",matchTarget);
          }
        }break;
        case ntRoute:
        {
          switch(matchTarget)
          {
            case mtId:return MatchField(sms.routeId)?mrMatched:mrNotMatched;
            case mtServiceId:
            {
              char buf[32];
              sprintf(buf,"%d",sms.serviceId);
              return MatchField(buf)?mrMatched:mrNotMatched;
            }
            case mtProviderId:
            {
              char buf[32];
              sprintf(buf,"%d",sms.providerId);
              return MatchField(buf)?mrMatched:mrNotMatched;
            }
            default:throw Exception("ntRoute - unknown matchTarget %d",matchTarget);
          }
        }break;
        case ntOr:
        {
          __trace__("or");
          Node* orNode=childNode;
          while(orNode)
          {
            if(orNode->Match(sms)==mrMatched)return mrMatched;
            orNode=orNode->nextNode;
          }
          return mrNotMatched;
        }
        case ntAnd:
        {
          __trace__("and");
          Node* orNode=childNode;
          while(orNode)
          {
            if(orNode->Match(sms)!=mrMatched)return mrNotMatched;
            orNode=orNode->nextNode;
          }
          return mrMatched;
        }
        case ntBill:
        {
          if(billServiceId!=-1)
          {
            sms.setServiceId(billServiceId);
          }
          return mrBilled;
        }
        case ntNobill:
        {
          return mrNotBilled;
        }
        default:throw Exception("Unknown node type %d",nodeType);
      }
      return mrNotMatched;
    }

  };

  Node* rootNode;

  friend class NodeBuilder;
};

class BillingRulesManager{
public:
  static void Init(const char* fileName);
  static void Reload(const char* fileName);
  static void Uninit();

  void Load(const char* fileName);

  class BRMReferer;
  friend class BRMReferer;
  class BRMReferer{
  public:
    BRMReferer(BillingRulesManager* argBrm):brm(argBrm)
    {
      brm->Ref();
    }
    BRMReferer(const BRMReferer& regArg):brm(regArg.brm)
    {
      brm->Ref();
    }
    ~BRMReferer()
    {
      brm->Unref();
    }
    BillingRulesManager* operator->()
    {
      return brm;
    }
  protected:
    BillingRulesManager* brm;
  };

  /*


    INTEFACE


  */

  static BRMReferer getInstance()
  {
    MutexGuard mg(instanceMtx);
    return BRMReferer(instancePtr);
  }

  const BillingRule& getBillingRule(const char* name)const
  {
    BillingRule** brPtr=rules.GetPtr(name);
    if(brPtr)
    {
      return **brPtr;
    }else
    {
      throw Exception("Billing rule %s not found",name);
    }
  }

  /*

    END OF INTERFACE

  */

protected:
  static BillingRulesManager* instancePtr;
  static Mutex instanceMtx;

  mutable volatile int refCount;
  mutable Mutex refMtx;

  BillingRulesManager()
  {
    refCount=0;
  }

  void Ref()const
  {
    MutexGuard g(refMtx);
    refCount++;
  }
  void Unref()const
  {
    int cntVal;
    {
      MutexGuard g(refMtx);
      cntVal=--refCount;
    }
    if(cntVal==0)
    {
      delete this;
    }
  }

  ~BillingRulesManager()
  {
    __trace__("~BillingRulesManager()");
    rules.First();
    char* key;
    BillingRule* val;
    while(rules.Next(key,val))
    {
      delete val;
    }
  }

  smsc::core::buffers::Hash<BillingRule*> rules;
};


}//namespace rules
}//namespace billing
}//namespace smppgw
}//namespace smsc


#endif
