/*
        $Id$
*/

#if !defined __Cpp_Header__router_types_h__
#define __Cpp_Header__router_types_h__

#include "smeman/smetypes.h"
#include "acls/interfaces.h"
#include <string>
#include "sms/sms.h"

namespace smsc {
namespace router {

typedef int RoutePriority;
static const RoutePriority RoutePriorityMax = 32000;
static const RoutePriority RoutePriorityMin = 0;

typedef enum {ReplyPathPass, ReplyPathForce, ReplyPathSuppress} ReplyPath;

typedef std::string RouteId;
typedef std::string BillingId;

struct TrafficRules{
  enum{
    limitDenied,
    limitNoLimit,
    limitPerHour,
    limitPerDay,
    limitPerWeek,
    limitPerMonth
  };
  bool allowIncom;
  bool allowAnswer;
  int  sendLimit; //sms per period
  int  limitType; //length of period
  bool allowPssrResp;
  bool allowUssrRequest;
  int  miUssdSessionLimit;
  bool allowInitiateUssdSession;
  int  siUssdSessionLimit;
  TrafficRules()
  {
    allowIncom=false;
    allowAnswer=false;
    sendLimit=0;
    limitType=0;
    allowPssrResp=false;
    allowUssrRequest=false;
    miUssdSessionLimit=0;
    allowInitiateUssdSession=false;
    siUssdSessionLimit=0;
  }
  TrafficRules(const std::string& str)
  {
    using std::string;
    string::size_type oldpos=0,pos=0,argpos;
    string opt,arg;
    //__trace2__("parsing trafic rules: %s",str.c_str());
    do{
      oldpos=pos==0?pos:pos+1;
      pos=str.find(',',oldpos);
      opt=str.substr(oldpos,pos==string::npos?pos:pos-oldpos);
      argpos=opt.find('=');
      if(argpos!=string::npos)
      {
        arg=opt.substr(argpos+1);
        opt.erase(argpos);
      }else
      {
        arg="";
      }
      if(opt.length()==0)
      {
        //
      }else
      if      (opt=="allow_receive")
      {
        allowIncom=true;
      }else if(opt=="allow_answer")
      {
        allowAnswer=true;
      }else if(opt=="send_limit")
      {
        if(arg.length()>0)
        {
          switch(arg[arg.length()-1])
          {
            case 'U':limitType=limitNoLimit;break;
            case 'H':limitType=limitPerHour;break;
            case 'D':limitType=limitPerDay;break;
            case 'W':limitType=limitPerWeek;break;
            case 'M':limitType=limitPerMonth;break;
            default:limitType=limitPerDay;break;
          }
          sendLimit=atoi(arg.c_str());
        }else
        {
          sendLimit=0;
          limitType=limitDenied;
        }
      }else if(opt=="allow_pssr_resp")
      {
        allowPssrResp=true;
      }else if(opt=="allow_ussr_request")
      {
        allowUssrRequest=true;
      }else if(opt=="ussd_mi_dialog_limit")
      {
        miUssdSessionLimit=atoi(arg.c_str());
      }else if(opt=="allow_ussd_dialog_init")
      {
        allowInitiateUssdSession=true;
      }else if(opt=="ussd_si_dialog_limit")
      {
        siUssdSessionLimit=atoi(arg.c_str());
      }else
      {
        __warning2__("Unknown route traffic rule: %s",opt.c_str());
      }
    }while(pos!=string::npos);
    /*
    __trace2__("result: "
               "allowIncom:%s"
               ",allowAnswer:%s"
               ",sendLimit:%d"
               ",limitType:%d"
               ",allowPssrResp:%s"
               ",allowUssrRequest:%s"
               ",miUssdSessionLimit:%d"
               ",allowInitiateUssdSession:%s"
               ",siUssdSessionLimit:%d",
               allowIncom?"Y":"N",
               allowAnswer?"Y":"N",
               sendLimit,
               limitType,
               allowPssrResp?"Y":"N",
               allowUssrRequest?"Y":"N",
               miUssdSessionLimit,
               allowInitiateUssdSession?"Y":"N",
               siUssdSessionLimit
              );
    */
  }
};

struct RouteInfo
{
  RoutePriority priority;
  uint8_t billing;
  bool archived;
  bool enabling;
  bool suppressDeliveryReports;
  bool hide;
  bool forceDelivery;
  bool transit;
  bool allowBlocked;
  ReplyPath replyPath;
  int serviceId;
  smsc::acls::AclIdent aclId;
  RouteId routeId;
  smsc::smeman::SmeSystemId smeSystemId;
  smsc::smeman::SmeSystemId srcSmeSystemId;
  smsc::sms::Address source;
  smsc::sms::Address dest;
  uint8_t deliveryMode;
  std::string forwardTo;
  std::string srcSubj; // for duplucate route debugging purposes only
  std::string dstSubj;
  std::string backupSme;
  TrafficRules trafRules;
  int32_t providerId;
  BillingId billingId;
  int32_t categoryId;
};

} // namespace router
} // namespcase smsc

#endif
