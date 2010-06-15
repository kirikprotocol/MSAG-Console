#include <exception>

#include "logger/Logger.h"
#include "util/debug.h"
#include "util/udh.hpp"
#include "util/recoder/recode_dll.h"
#include "util/smstext.h"
#include "util/regexp/RegExp.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include "inman/storage/cdrutil.hpp"
#include "smsc/closedgroups/ClosedGroupsInterface.hpp"
#include "smsc/status.h"
#include "smsc/resourcemanager/ResourceManager.hpp"
#include "smsc/smsc.hpp"
#include "smsc/common/rescheduler.hpp"
#include "smsc/profiler/profiler.hpp"
#include "smsc/common/TimeZoneMan.hpp"
#ifdef SMSEXTRA
#include "smsc/extra/Extra.hpp"
#include "smsc/extra/ExtraBits.hpp"
#endif

#include "constants.hpp"
#include "util.hpp"
#include "state_machine.hpp"


#ident "@(#)$Id$"


namespace smsc{
namespace statemachine{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::snmp;
using namespace smsc::profiler;
using namespace smsc::common;
using namespace smsc::resourcemanager;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using smsc::core::synchronization::Mutex;

static time_t parseReceiptTime(const std::string& txt,SMatch* m,int idx)
{
  int YY,MM,DD,hh,mm;
  sscanf(txt.c_str()+m[idx].start,"%02d%02d%02d%02d%02d",&YY,&MM,&DD,&hh,&mm);
  YY+=2000;
  tm t={0,};
  t.tm_year=YY-1900;
  t.tm_mon=MM-1;
  t.tm_mday=DD;
  t.tm_hour=hh;
  t.tm_min=mm;
  return mktime(&t);
}


StateType StateMachine::submit(Tuple& t)
{
  __require__(t.state==UNKNOWN_STATE || t.state==ENROUTE_STATE || t.state==ERROR_STATE);

  SbmContext c(t);

  c.src_proxy=t.command.getProxy();

  __require__(c.src_proxy!=NULL);

  SMS* sms = t.command->get_sms();
  c.sms=sms;

  sms->setSourceSmeId(t.command->get_sourceId());

  if(!sms->Invalidate(__FILE__,__LINE__))
  {
    warn2(smsLog, "Invalidate of %lld failed\n",t.msgId);
    submitResp(t,sms,Status::SUBMITFAIL);
    return ERROR_STATE;
  }

  c.dialogId =  t.command->get_dialogId();
  sms->dialogId=c.dialogId;

  debug2(smsLog, "SBM: Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
    t.msgId,c.dialogId,
    sms->getOriginatingAddress().toString().c_str(),
    sms->getDestinationAddress().toString().c_str(),
    c.src_proxy->getSystemId()
  );

  if(t.state==ERROR_STATE)
  {
    warn1(smsLog, "SMS in error state\n");
    submitResp(t,sms,Status::SUBMITFAIL);
    return ERROR_STATE;
  }

  // check scheduler limit
  if(smsc->checkSchedulerHardLimit())
  {
    submitResp(t,sms,Status::SCHEDULERLIMIT);
    info2(smsLog,"SBM: sms denied by hard scheduler limit oa=%s;da=%s;srcprx=%s",
          sms->getOriginatingAddress().toString().c_str(),
          sms->getDestinationAddress().toString().c_str(),
          c.src_proxy->getSystemId());
    return ERROR_STATE;
  }

  ////
  //
  //  SMS validity checks started
  //

  if(sms->getNextTime()==-1)
  {
    submitResp(t,sms,Status::INVSCHED);
    warn2(smsLog, "SBM: invalid schedule time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)) {
    // force forward mode
    sms->setIntProperty(Tag::SMPP_ESM_CLASS, (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0xFC)|0x02);
  }

  if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
     sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
     sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0)
  {
    submitResp(t,sms,Status::SUBMITFAIL);

    warn2(smsLog, "SBM: both short_message and payload present Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::LATIN1 &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::UCS2 &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::BINARY &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::SMSC7BIT)
  {
    submitResp(t,sms,Status::INVDCS);
    warn2(smsLog, "SBM: invalid datacoding %d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM) &&
     (
       sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)<1 ||
       sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)>sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)
     )
    )
  {
    submitResp(t,sms,Status::INVOPTPARAMVAL);
    warn2(smsLog, "SBM: invalid SMPP_SAR_SEGMENT_SEQNUM %d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    unsigned len;
    const unsigned char* msg;
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    }
    else
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
    if(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
    {
      len-=*msg+1;
    }
    ////
    // Unicode message with odd length
    if(len&1)
    {
      submitResp(t,sms,Status::INVMSGLEN);
      warn2(smsLog, "SBM: invalid message length for unicode (%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
        len,
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId()
      );
      return ERROR_STATE;
    }
  }


  if(sms->getValidTime()==-1)
  {
    submitResp(t,sms,Status::INVEXPIRY);
    warn2(smsLog, "SBM: invalid valid time Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(c.src_proxy->getSourceAddressRange().length() &&
     !checkSourceAddress(c.src_proxy->getSourceAddressRange(),sms->getOriginatingAddress()))
  {
    submitResp(t,sms,Status::INVSRCADR);
    warn2(smsLog, "SBM: invalid source address for range '%s' Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      c.src_proxy->getSourceAddressRange().c_str(),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if((
      (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1 ||
      (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2
     ) &&
     sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    submitResp(t,sms,Status::SUBMITFAIL);
    warn2(smsLog, "SBM: attempt to send concatenated sms in dg or tr mode. Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  if(sms->originatingDescriptor.mscLength || sms->originatingDescriptor.imsiLength)
  {
    char buf[MAX_ADDRESS_VALUE_LENGTH+MAX_ADDRESS_VALUE_LENGTH+12];
    sprintf(buf,"Org: %s/%s",sms->originatingDescriptor.msc,sms->originatingDescriptor.imsi);
    sms->setStrProperty(Tag::SMSC_DESCRIPTORS,buf);
  }


  time_t now=time(NULL);
  sms->setSubmitTime(now);

  // route sms
  //SmeProxy* c.dest_proxy = 0;
  smsc_log_debug(smsLog,"AliasToAddress: %s",sms->getDestinationAddress().toString().c_str());
  if(smsc->AliasToAddress(sms->getDestinationAddress(),c.dst))
  {
    smsc_log_debug(smsLog,"ALIAS:%s->%s",sms->getDestinationAddress().toString().c_str(),c.dst.toString().c_str());
  }
  else
  {
    c.dst=sms->getDestinationAddress();
  }
  sms->setDealiasedDestinationAddress(c.dst);
  c.profile=smsc->getProfiler()->lookup(sms->getOriginatingAddress());
  Profile& profile=c.profile;

  if(profile.closedGroupId!=0 && !smsc::closedgroups::ClosedGroupsInterface::getInstance()->Check(profile.closedGroupId,sms->getDealiasedDestinationAddress()))
  {
    info2(smsLog,"SBM: Id=%lld, denied by closed group(%d:'%s') check",
      t.msgId,profile.closedGroupId,
      smsc::closedgroups::ClosedGroupsInterface::getInstance()->GetClosedGroupName(profile.closedGroupId)
    );
    submitResp(t,sms,Status::DENIEDBYCLOSEDGROUP);
    return ERROR_STATE;
  }

  smsc::profiler::Profile orgprofile=profile;

  if((sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x03)==0x01 ||
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x03)==0x02 ||
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x10)==0x10 ||
     sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST)
    )
  {
    sms->setDeliveryReport(0);
  }else
  {
    sms->setDeliveryReport(profile.reportoptions);
  }

  if(sms->getDeliveryReport()==0 &&
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x03)==0 &&
     (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x10)==0 &&
     sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
  {
    sms->setDeliveryReport(ProfileReportOptions::ReportFinal);
  }

  sms->setIntProperty(Tag::SMSC_HIDE,profile.hide);
  if(profile.hide==HideOption::hoSubstitute)
  {
    Address addr;
    if(smsc->AddressToAlias(sms->getOriginatingAddress(),addr))
    {
      info2(smsLog,"Id=%lld: oa subst: %s->%s",t.msgId,sms->getOriginatingAddress().toString().c_str(),addr.toString().c_str());
      sms->setOriginatingAddress(addr);
    }
  }

  if(!sms->hasIntProperty(Tag::SMSC_TRANSLIT))
  {
    sms->setIntProperty(Tag::SMSC_TRANSLIT,profile.translit);
    debug2(smsLog,"Id=%lld, set translit to %d",t.msgId,sms->getIntProperty(Tag::SMSC_TRANSLIT));
  }

  if(sms->getIntProperty(Tag::SMPP_SET_DPF) && (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)!=2)
  {
    sms->setIntProperty(Tag::SMPP_SET_DPF,0);
  };


  c.srcprof=profile;

  int profileMatchType;
  std::string profileMatchAddress;
  profile=smsc->getProfiler()->lookupEx(c.dst,profileMatchType,profileMatchAddress);

  if(profileMatchType==ProfilerMatchType::mtExact)
  {
    debug2(smsLog,"Id=%lld exact profile match, set translit to 1",t.msgId);
    sms->setIntProperty(Tag::SMSC_TRANSLIT,1);
  }

  if(!smsCanBeTransliterated(sms))
  {
    debug2(smsLog,"Id=%lld cannot be transliterated, set translit to 0",t.msgId);
    sms->setIntProperty(Tag::SMSC_TRANSLIT,0);
  }

  if(smsc->isNoDivert(sms->getSourceSmeId()))
  {
    profile.divert="";
  }

  bool diverted=false;

  int divertFlags=(profile.divertActive        ?DF_UNCOND:0)|
                  (profile.divertActiveAbsent  ?DF_ABSENT:0)|
                  (profile.divertActiveBlocked ?DF_BLOCK :0)|
                  (profile.divertActiveBarred  ?DF_BARRED:0)|
                  (profile.divertActiveCapacity?DF_CAPAC :0);
  if(divertFlags && profile.divert.length()!=0 &&
     !sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) &&
     sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=3
    )
  {
    debug2(smsLog, "divert for %s found",c.dst.toString().c_str());
    Address divDst;
    try{
      divDst=Address(profile.divert.c_str());
    }catch(...)
    {
      warn2(smsLog, "INVALID DIVERT FOR %s - ADDRESS:%s",c.dst.toString().c_str(),profile.divert.c_str());
      goto divert_failed;
    }
    Address tmp;
    if(smsc->AliasToAddress(divDst,tmp))
    {
      debug2(smsLog, "Divert address dealiased:%s->%s",divDst.toString().c_str(),tmp.toString().c_str());
      divDst=tmp;
    }
    smsc::router::RouteInfo ri2;
    SmeProxy* prx;
    int idx;
    try{
      if(smsc->routeSms(sms->getOriginatingAddress(),divDst,idx,prx,&ri2,c.src_proxy->getSmeIndex()))
      {
        if(ri2.smeSystemId!="MAP_PROXY")
        {
          warn2(smsLog,"attempt to divert to non-map address(sysId=%s):%s->%s",
            ri2.smeSystemId.c_str(),
            sms->getOriginatingAddress().toString().c_str(),divDst.toString().c_str());
          goto divert_failed;
        }
      }
    }catch(std::exception& e)
    {
      warn2(smsLog,"routing failed during divert check:%s",e.what());
      goto divert_failed;
    }
    sms->setStrProperty(Tag::SMSC_DIVERTED_TO,divDst.toString().c_str());
    if(divertFlags&DF_UNCOND)
    {
      diverted=true;
      c.dst=divDst;
    }

    Profile p=smsc->getProfiler()->lookup(divDst);

    divertFlags|=p.udhconcat?DF_UDHCONCAT:0;
    int ddc=p.codepage;
    if(!(profile.codepage&DataCoding::UCS2) && (ddc&DataCoding::UCS2))
    {
      ddc=profile.codepage;
      smsc_log_debug(smsLog,"divert - downgrade dstdc to %d",ddc);
    }

    divertFlags|=(ddc)<<DF_DCSHIFT;

    if(divertFlags&DF_UNCOND)profile=p;

    sms->setIntProperty(Tag::SMSC_DIVERTFLAGS,divertFlags);

  }
  divert_failed:;

  sms->setIntProperty(Tag::SMSC_ORIGINALPARTSNUM,1);



  ////
  //
  //  Routing here
  //

  c.has_route = false;

  try{
    c.has_route=smsc->routeSms(sms->getOriginatingAddress(),
                            c.dst,
                            c.dest_proxy_index,c.dest_proxy,&c.ri,c.src_proxy->getSmeIndex());
  }catch(std::exception& e)
  {
    warn2(smsLog,"Routing %s->%s failed:%s",sms->getOriginatingAddress().toString().c_str(),
      c.dst.toString().c_str(),e.what());
  }

  if ( !c.has_route )
  {
    submitResp(t,sms,Status::NOROUTE);
    warn2(smsLog, "SBM: no route Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.src_proxy->getSystemId()
    );
    return ERROR_STATE;
  }

  c.generateDeliver=true; // do not generate in case of merge-concat
  c.createSms=scsCreate;

  c.needToSendResp=true;

  c.noPartitionSms=false; // do not call partitionSms if true!

  c.fromMap=c.src_proxy && !strcmp(c.src_proxy->getSystemId(),"MAP_PROXY");
  c.toMap=c.dest_proxy && !strcmp(c.dest_proxy->getSystemId(),"MAP_PROXY");
  c.fromDistrList=c.src_proxy && !strcmp(c.src_proxy->getSystemId(),"DSTRLST");

  SmeInfo dstSmeInfo=smsc->getSmeInfo(c.dest_proxy_index);

  if(c.toMap && ((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3c)==0x08 ||
      (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3c)==0x10))
  {
    std::string txt;
    getSmsText(sms,txt,CONV_ENCODING_CP1251);
    SMatch m[10];
    int n=10;
    if(smeAckRx.Match(txt.c_str(),txt.c_str()+txt.length(),m,n))
    {
      time_t sbmTime=parseReceiptTime(txt,m,1);
      time_t dlvTime=parseReceiptTime(txt,m,2);
      std::string st=txt.substr(m[3].start,m[3].end-m[3].start);
      int err;
      sscanf(txt.c_str()+m[4].start,"%d",&err);
      FormatData fd;
      char addr[32];
      sms->getOriginatingAddress().getText(addr,sizeof(addr));
      fd.addr=addr;
      fd.ddest=addr;
      fd.submitDate=sbmTime;
      fd.date=dlvTime;
      fd.setLastResult(err);
      fd.locale=profile.locale.c_str();
      fd.msc="";
      fd.msgId="";
      fd.scheme=dstSmeInfo.receiptSchemeName.c_str();
      std::string out;
      fd.err="";
      smsc_log_debug(smsLog,"esme delivery ack detected in Id=%lld. sbmTime=%lu dlvTime=%lu, st=%s",t.msgId,sbmTime,dlvTime,st.c_str());
      if(st=="DELIVRD")
      {
        formatDeliver(fd,out);
      }else if(st=="ACCEPTD")
      {
        formatNotify(fd,out);
      }else
      {
        fd.err=txt.c_str()+m[5].start;
        formatFailed(fd,out);
      }
      fillSms(sms,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);
      if(sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
      {
        sms->getMessageBody().dropIntProperty(Tag::SMSC_ORIGINAL_DC);
        sms->getMessageBody().dropIntProperty(Tag::SMSC_FORCE_DC);
      }
    }
  }

  sms->setArchivationRequested(c.ri.archived);

#ifdef SMSEXTRA
  /*
  if(sms->getIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    unsigned char *body;
    unsigned int len;
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      body=(unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    }else
    {
      body=(unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
    uint16_t mr;
    uint8_t idx,num;
    bool havemoreudh;
    smsc::util::findConcatInfo(body,mr,idx,num,havemoreudh);
    if(idx!=1)
    {
      firstPart=false;
    }
  }
  */
  if(!ExtraProcessing(c))
  {
    return ERROR_STATE;
  }
#endif

  bool dropMergeConcat=false;

  if(c.ri.transit)
  {
    dropMergeConcat=true;
  }

  c.isForwardTo = false;
  if( c.ri.forwardTo.length() > 0 && c.toMap )
  {
    sms->setStrProperty( Tag::SMSC_FORWARD_MO_TO, c.ri.forwardTo.c_str());

    // force forward(transaction) mode
    sms->setIntProperty( Tag::SMPP_ESM_CLASS, sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x02 );
    c.isForwardTo = true;

    dropMergeConcat=true;
    sms->getMessageBody().dropProperty(Tag::SMSC_DC_LIST);
  }

  if(dropMergeConcat)
  {
    if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
    {
      dropMergeConcat=true;
      unsigned char *body;
      unsigned int len;
      if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        body=(unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      }else
      {
        body=(unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
      }
      uint16_t mr;
      uint8_t idx,num;
      bool havemoreudh;
      smsc::util::findConcatInfo(body,mr,idx,num,havemoreudh);

      smsc->submitMrKill(sms->getOriginatingAddress(),sms->getDestinationAddress(),mr);
      sms->getMessageBody().dropIntProperty(Tag::SMSC_MERGE_CONCAT);
    }
  }


  if(!c.ri.transit)
  {
    if(!extactConcatInfoToSar(*sms))
    {
      warn2(smsLog,"extactConcatInfoToSar failed. Id=%lld;oa=%s;da=%s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    }


    if(!extractPortsFromUdh(*sms))
    {
      warn2(smsLog,"extractPortsFromUdh failed. Id=%lld;oa=%s;da=%s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    }


    if(!convertSarToUdh(*sms))
    {
      warn2(smsLog,"convertSarToUdh failed. Id=%lld;oa=%s;da=%s",
                            t.msgId,
                            sms->getOriginatingAddress().toString().c_str(),
                            sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::INVOPTPARAMVAL);
      return ERROR_STATE;
    };
  }
  ////
  //
  //  Merging
  //

  if(sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
  {
    if(!processMerge(c))
    {
      return c.rvstate;
    }
  }

  //
  //  End of merging
  //
  ////

  sms->setRouteId(c.ri.routeId.c_str());
  if(c.ri.suppressDeliveryReports)sms->setIntProperty(Tag::SMSC_SUPPRESS_REPORTS,1);
  int prio=sms->getPriority()+c.ri.priority;
  if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
  sms->setPriority(prio);

  debug2(smsLog,"SBM: route %s->%s found:%s",
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.ri.routeId.c_str());


  sms->setIntProperty(Tag::SMSC_PROVIDERID,c.ri.providerId);
  sms->setIntProperty(Tag::SMSC_CATEGORYID,c.ri.categoryId);

  bool aclCheck=false;
  smsc::core::buffers::FixedLengthString<32> aclAddr;
  //std::string aclAddr;

  bool& fromMap=c.fromMap;
  bool& toMap=c.toMap;

  if(c.toMap)
  {
    if(sms->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND))
    {
      debug1(smsLog,"drop SMPP_MORE_MESSAGES_TO_SEND");
      sms->getMessageBody().dropIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND);
    }
  }

  if((fromMap || toMap) && !(fromMap && toMap))
  {
    char buf[MAX_ADDRESS_VALUE_LENGTH];
    if(fromMap)
    {
      sms->getOriginatingAddress().getText(buf,sizeof(buf));
    }else
    {
      sms->getDestinationAddress().getText(buf,sizeof(buf));
    }
    aclAddr=(const char*)buf;
    aclCheck=true;
  }

  if(c.ri.billing==smsc::sms::BILLING_ONSUBMIT || c.ri.billing==smsc::sms::BILLING_CDR)
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnSubmit);
  }else
  if((fromMap || c.fromDistrList) && toMap)//peer2peer
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->p2pChargePolicy);
  }else
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->otherChargePolicy);
  }

#ifdef SMSEXTRA
  if(c.ri.billing==smsc::sms::BILLING_MT)
  {
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnSubmit);
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_INCHARGE);
  }
#endif
  if(c.ri.deliveryMode==SMSC_TRANSACTION_MSG_MODE || (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
  {
    // set charge on delivery to avoid charging of sms that could be undelivered
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnDelivery);
  }


  if(aclCheck && c.ri.aclId!=-1 && !smsc->getAclMgr()->isGranted(c.ri.aclId,aclAddr.c_str()))
  {
    submitResp(t,sms,Status::NOROUTE);
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
    c.dst.toString(buf2,sizeof(buf2));
    warn2(smsLog, "SBM: acl access denied (aclId=%d) Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
      c.ri.aclId,
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
      c.src_proxy->getSystemId()
    );
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

  if(toMap)
  {
    if(c.ri.forceDelivery)
    {
      sms->setIntProperty(Tag::SMPP_PRIORITY,3);
    }
  }

  if(fromMap && ( sms->getOriginatingDescriptor().mscLength==0 ||
                  sms->getOriginatingDescriptor().imsiLength==0 ))
  {
    if(!c.ri.allowBlocked)
    {
      submitResp(t,sms,Status::CALLBARRED);
      char buf1[32];
      char buf2[32];
      sms->getOriginatingAddress().toString(buf1,sizeof(buf1));
      c.dst.toString(buf2,sizeof(buf2));
      warn2(smsLog, "SBM: call barred Id=%lld;seq=%d;oa=%s;%s;srcprx=%s",
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        AddrPair("da",sms->getDestinationAddress(),"dda",c.dst).c_str(),
        c.src_proxy->getSystemId()
      );
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }
  }

  smsc_log_debug(smsLog,"dst sme '%s', flags=%x",dstSmeInfo.systemId.c_str(),dstSmeInfo.flags);

#ifdef SMSEXTRA

  SmeInfo srcSmeInfo=smsc->getSmeInfo(c.src_proxy->getSystemId());

  if(srcSmeInfo.SME_N==EXTRA_GROUPS)
  {
    smsc_log_debug(smsLog,"added EXTRA_GROUPS to SMSC_EXTRAFLAGS from smeN");
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_GROUPS);
  }

  /*
  if((sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_NICK) &&
     (
       !dstSmeInfo.wantAlias ||
       !ri.hide
     )
    )
  {
    info2(smsLog,"EXTRA: smsnick not allowed for route %s",ri.routeId.c_str());
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&~EXTRA_NICK);
  }
  */
  /*
  NO LONGER NEEDED!
  Sponsored uses another scheme.

  if(fromMap && toMap && c.srcprof.sponsored>0)
  {
    info2(smsLog,"EXTRA: sponsored sms for abonent %s(cnt=%d)",sms->getOriginatingAddress().toString().c_str(),c.srcprof.sponsored);
    sms->setIntProperty(Tag::SMSC_EXTRAFLAGS,sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)|EXTRA_SPONSORED);
    smsc->getProfiler()->decrementSponsoredCount(sms->getOriginatingAddress());
  }
  */
#endif

  if((dstSmeInfo.accessMask&c.srcprof.accessMaskOut)==0)
  {
    info2(smsLog,"SBM: Id=%lld, denied by access out mask (%s=%x,%s=%x",t.msgId,dstSmeInfo.systemId.c_str(),dstSmeInfo.accessMask,sms->getOriginatingAddress().toString().c_str(),c.srcprof.accessMaskOut);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

  if((c.src_proxy->getAccessMask()&profile.accessMaskIn)==0)
  {
    info2(smsLog,"SBM: Id=%lld, denied by access in mask(%s=%x,%s=%x",t.msgId,c.src_proxy->getSystemId(),c.src_proxy->getAccessMask(),sms->getDestinationAddress().toString().c_str(),profile.accessMaskIn);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }

  if((c.srcprof.accessMaskOut&profile.accessMaskIn)==0)
  {
    info2(smsLog,"SBM: Id=%lld, denied by access masks(%s=%x,%s=%x",t.msgId,sms->getOriginatingAddress().toString().c_str(),c.srcprof.accessMaskOut,sms->getDestinationAddress().toString().c_str(),profile.accessMaskIn);
    submitResp(t,sms,Status::DENIEDBYACCESSMASK);
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }
  if(dstSmeInfo.hasFlag(sfCarryOrgAbonentInfo))
  {
    sms->setStrProperty(Tag::SMSC_SUPPORTED_LOCALE,orgprofile.locale.c_str());
    sms->setIntProperty(Tag::SMSC_SUPPORTED_CODESET,orgprofile.codepage);
  }
  if(dstSmeInfo.hasFlag(sfCarryOrgDescriptor))
  {
    if(sms->getOriginatingDescriptor().imsiLength)
    {
      sms->setStrProperty(Tag::SMSC_IMSI_ADDRESS,sms->getOriginatingDescriptor().imsi);
    }
    if(sms->getOriginatingDescriptor().mscLength)
    {
      sms->setStrProperty(Tag::SMSC_MSC_ADDRESS,sms->getOriginatingDescriptor().msc);
    }
  }

#ifdef SMSEXTRA
  {
    if(c.toMap && srcSmeInfo.hasFlag(sfFillExtraDescriptor) && sms->hasStrProperty(Tag::SMSC_MSC_ADDRESS))
    {
      smsc_log_debug(smsLog,"Filling descriptor from smpp fields:%s/%s",sms->getStrProperty(Tag::SMSC_IMSI_ADDRESS).c_str(),sms->getStrProperty(Tag::SMSC_MSC_ADDRESS).c_str());
      Descriptor d;
      const SmsPropertyBuf& imsi(sms->getStrProperty(Tag::SMSC_IMSI_ADDRESS));
      d.setImsi((uint8_t)imsi.length(),imsi.c_str());
      const SmsPropertyBuf& msc(sms->getStrProperty(Tag::SMSC_MSC_ADDRESS));
      d.setMsc((uint8_t)msc.length(),msc.c_str());
      sms->setOriginatingDescriptor(d);
    }
  }
#endif


  sms->setDestinationSmeId(c.ri.smeSystemId.c_str());
  sms->setServiceId(c.ri.serviceId);


  smsc_log_debug(smsLog,"SUBMIT: archivation request for Id=%lld;seq=%d is %s",t.msgId,c.dialogId,c.ri.archived?"true":"false");

#ifdef SMSEXTRA
  if(sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_FAKE)
  {
    sms->setBillingRecord(BILLING_NONE);
  }
  if(!(sms->getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_NICK))
  {
    sms->setBillingRecord(c.ri.billing);
  }
#else
  sms->setBillingRecord(c.ri.billing);
#endif


  sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);

  if(c.ri.smeSystemId=="MAP_PROXY" && sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    sms->setIntProperty(Tag::SMSC_TRANSLIT,1);
    if(profile.codepage&smsc::profiler::ProfileCharsetOptions::UssdIn7Bit)
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,smsc::profiler::ProfileCharsetOptions::Default);
    }else
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,smsc::profiler::ProfileCharsetOptions::Ucs2);
    }
  }else
  {
    sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage&(~smsc::profiler::ProfileCharsetOptions::UssdIn7Bit));
  }

  sms->setIntProperty(Tag::SMSC_UDH_CONCAT,profile.udhconcat);


  int pres=psSingle;


  if((sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime) && !sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) &&
     (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)!=0x2)
  {
    sms->setValidTime(now+maxValidTime);
    debug2(smsLog,"maxValidTime=%d",maxValidTime);
  }

  debug2(smsLog,"Valid time for sms Id=%lld:%u",t.msgId,(unsigned int)sms->getValidTime());



  if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
  {
    submitResp(t,sms,Status::INVSCHED);
    warn2(smsLog, "SBM: invalid schedule time(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s",
      sms->getNextTime(),
      t.msgId,c.dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      c.src_proxy->getSystemId()
    );
    if(c.createSms==scsReplace)
    {
      try{
        store->changeSmsStateToDeleted(t.msgId);
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
      }
    }
    return ERROR_STATE;
  }


  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


  if( !c.isForwardTo && !c.ri.transit)
  {

    sms->getMessageBody().dropProperty(Tag::SMSC_MO_PDU);

    ////
    //
    // Override delivery mode if specified in config and default mode in sms
    //

    if( c.ri.deliveryMode != smsc::sms::SMSC_DEFAULT_MSG_MODE)
    {
      if(sms->hasBinProperty(Tag::SMSC_CONCATINFO) && c.ri.deliveryMode!=smsc::sms::SMSC_STOREANDFORWARD_MSG_MODE)
      {
        smsc_log_warn(smsLog,"Attempt to send multipart message in forward mode with route '%s'",c.ri.routeId.c_str());
      }else
      {
        int esmcls = sms->getIntProperty( Tag::SMPP_ESM_CLASS );
        // following if removed at 25.09.2006 by request of customers
        //if( (esmcls&0x3) == smsc::sms::SMSC_DEFAULT_MSG_MODE )
        {
          // allow override
          sms->setIntProperty( Tag::SMPP_ESM_CLASS, (esmcls&~0x03)|(c.ri.deliveryMode&0x03) );
          isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
          isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;
        }
      }
    }

    if((isDatagram || isTransaction) && sms->hasBinProperty(Tag::SMSC_DC_LIST))
    {
      try
      {
        smsc_log_debug(smsLog,"SBM:Delete fwd/dgm mode merged Id=%lld from store",t.msgId);
        c.createSms=scsDoNotCreate;
        store->changeSmsStateToDeleted(t.msgId);
      } catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"SBM: merged miltipart->forward: failed to change Id=%lld to deleted",t.msgId);
      }
    }

    ////
    //
    //  Directives
    //

    try{
      if(!sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        processDirectives(*sms,profile,c.srcprof);
      }
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"Failed to process directives due to exception:%s",e.what());
      submitResp(t,sms,Status::SUBMITFAIL);
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }

    if(sms->getIntProperty(Tag::SMSC_TRANSLIT)==0)
    {
      if(sms->getIntProperty(Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
      {
        debug2(smsLog,"Id=%lld translit set to 0, patch dstcodepage",t.msgId);
        sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2);
        if(sms->hasIntProperty(Tag::SMSC_DIVERTFLAGS))
        {
          int df=sms->getIntProperty(Tag::SMSC_DIVERTFLAGS);
          df&=~(0xff<<DF_DCSHIFT);
          df|=(DataCoding::UCS2)<<DF_DCSHIFT;
          sms->setIntProperty(Tag::SMSC_DIVERTFLAGS,df);
        }
      }
    }


    smsc_log_debug(smsLog,"SUBMIT_SM: after processDirectives - delrep=%d, sdt=%d",(int)sms->getDeliveryReport(),sms->getNextTime());

    if(c.ri.smeSystemId=="MAP_PROXY" && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03)
    {
      sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,DataCoding::UCS2|DataCoding::LATIN1);
      sms->setIntProperty(Tag::SMSC_UDH_CONCAT,1);
    }



    if(c.ri.smeSystemId=="MAP_PROXY" &&
       !sms->hasIntProperty(Tag::SMSC_MERGE_CONCAT)&&
       !sms->hasBinProperty(Tag::SMSC_CONCATINFO) &&
       !c.noPartitionSms
      )
    {
      pres=partitionSms(sms);
    }
    if(pres==psErrorLength)
    {
      submitResp(t,sms,Status::INVMSGLEN);
      unsigned int len;
      const char *msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      warn2(smsLog, "SBM: invalid message length(%d) Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        len,
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        c.ri.smeSystemId.c_str()
      );
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }

    if(pres==psErrorUdhi)
    {
      submitResp(t,sms,Status::SUBMITFAIL);
      warn2(smsLog, "SBM: udhi present in concatenated message!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,c.dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        c.ri.smeSystemId.c_str()
      );
      if(c.createSms==scsReplace)
      {
        try{
          store->changeSmsStateToDeleted(t.msgId);
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to change incomplete sms state to deleted Id=%lld",t.msgId);
        }
      }
      return ERROR_STATE;
    }

    if(pres==psErrorUssd)
    {
/*      submitResp(t,sms,Status::USSDMSGTOOLONG);
      warn2(smsLog, "SBM: ussd message too long!!! Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        c.src_proxy->getSystemId(),
        ri.smeSystemId.c_str()
      );
      return ERROR_STATE;*/
      pres = psSingle;
    }


    if(pres==psMultiple && !c.noPartitionSms)
    {
      uint8_t msgref=smsc->getNextMR(c.dst);
      sms->setConcatMsgRef(msgref);

      sms->setConcatSeqNum(0);
    }

    smsc_log_debug(smsLog,"SUBMIT_SM: SMPP_USSD_SERVICE_OP for Id=%lld:%d",t.msgId,sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));

    //
    // End of checks. Ready to put sms to database
    //
    ////

  }

  INSmsChargeResponse::SubmitContext ctx;
  ctx.srcProxy=c.src_proxy;
  ctx.dstProxy=c.dest_proxy;
  ctx.createSms=c.createSms;
  ctx.needToSendResp=c.needToSendResp;
  ctx.dialogId=c.dialogId;
  ctx.dest_proxy_index=c.dest_proxy_index;
  ctx.isForwardTo=c.isForwardTo;
  ctx.diverted=diverted;
  ctx.routeHide=c.ri.hide;
  ctx.dst=c.dst;
  ctx.transit=c.ri.transit;
  ctx.replyPath=c.ri.replyPath;
  ctx.priority=c.ri.priority;
  ctx.generateDeliver=c.generateDeliver;
  ctx.dstDlgIdx=t.command->dstNodeIdx;
  ctx.sourceId=t.command->sourceId;
#ifdef SMSEXTRA
  ctx.noDestChange=c.noDestChange;
#endif
  if(sms->billingRequired())
  {
    try{
      smsc->ChargeSms(t.msgId,*sms,ctx);
    }catch(std::exception& e)
    {
      submitResp(t,sms,Status::NOCONNECTIONTOINMAN);
      warn2(smsLog,"SBM:ChargeSms failed:%s",e.what());
      return ERROR_STATE;
    }
    return CHARGING_STATE;
  }else
  {
    Tuple t2;
    t2.msgId=t.msgId;
    t2.state=UNKNOWN_STATE;
    t2.command=SmscCommand::makeINSmsChargeResponse(t.msgId,*sms,ctx,1);
    return submitChargeResp(t2);
  }
}

StateType StateMachine::submitChargeResp(Tuple& t)
{
  INSmsChargeResponse* resp=t.command->get_chargeSmsResp();
  SMS* sms=&resp->sms;
  time_t stime=sms->getNextTime();
  time_t now=time(NULL);

  SmsCreationState createSms=(SmsCreationState)resp->cntx.createSms;
  bool needToSendResp=resp->cntx.needToSendResp;
  SmeProxy* src_proxy=resp->cntx.srcProxy;
  SmeProxy* dest_proxy=resp->cntx.dstProxy;
  int dialogId=resp->cntx.dialogId;
  int dest_proxy_index=resp->cntx.dest_proxy_index;
  bool isForwardTo=resp->cntx.isForwardTo;
  bool diverted=resp->cntx.diverted;
  bool routeHide=resp->cntx.routeHide;
  Address dst=resp->cntx.dst;
  bool transit=resp->cntx.transit;
  smsc::router::ReplyPath replyPath=resp->cntx.replyPath;
  int priority=resp->cntx.priority;
  t.command->dstNodeIdx=resp->cntx.dstDlgIdx;
  t.command->sourceId=resp->cntx.sourceId;
#ifdef SMSEXTRA
  bool noDestChange=resp->cntx.noDestChange;
#endif

  SmeInfo dstSmeInfo=smsc->getSmeInfo(dest_proxy_index);

  t.command.setProxy(src_proxy);
  t.command->set_dialogId(dialogId);
  sms->dialogId=dialogId;

  if(!resp->result)
  {
    submitResp(t,sms,Status::DENIEDBYINMAN);
    warn2(smsLog, "SBM: denied by inman Id=%lld;seq=%d;oa=%s;%s;srcprx=%s: '%s'",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms->getDestinationAddress(),"dda",dst).c_str(),
      src_proxy->getSystemId(),
      resp->inmanError.c_str()
    );
    return ERROR_STATE;
  }

#ifdef SMSEXTRA
  if(sms->billingRecord==BILLING_MT && resp->contractType!=smsc::inman::cdr::CDRRecord::abtPrepaid)
  {
    sms->setIntProperty(Tag::SMPP_ESM_CLASS,(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x3))|0x2);
    sms->setIntProperty(Tag::SMSC_CHARGINGPOLICY,Smsc::chargeOnDelivery);
  }
#endif

  ////
  //
  // Store sms to database
  //

  __require__(!(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
                sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
                sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0));

  bool isDatagram=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==1;
  bool isTransaction=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2;


  smsc_log_debug(smsLog,"SUBMIT: msgId=%lld, isDg=%s, isTr=%s",t.msgId,isDatagram?"true":"false",isTransaction?"true":"false");
  if(!isDatagram && !isTransaction && createSms==scsCreate)
  {
    try{
      if(sms->getNextTime()<now)
      {
        sms->setNextTime(now);
      }
      if(!sms->Invalidate(__FILE__,__LINE__))
      {
         warn2(smsLog, "Invalidate of %lld failed",t.msgId);
         throw Exception("Invalid sms");
      }
      bool rip=sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG)!=0;

      SMSId replaceId=store->createSms(*sms,t.msgId,rip?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
      if(rip && replaceId!=t.msgId)
      {
        smsc->getScheduler()->CancelSms(replaceId,sms->getDealiasedDestinationAddress());
      }

    }catch(...)
    {
      smsc_log_warn(smsLog,"failed to create sms with Id=%lld,oa=%s,da=%s",t.msgId,sms->getOriginatingAddress().toString().c_str(),sms->getDestinationAddress().toString().c_str());
      submitResp(t,sms,Status::SYSERR);
      smsc->ReportDelivery(resp->cntx.inDlgId,*sms,true,Smsc::chargeAlways);
      return ERROR_STATE;
    }
  }else if(createSms==scsReplace)
  {
    try
    {
      store->replaceSms(t.msgId,*sms);
    } catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"failed to create/replace sms with Id=%lld:%s",t.msgId,e.what());
      submitResp(t,sms,Status::SYSERR);
      smsc->ReportDelivery(resp->cntx.inDlgId,*sms,true,Smsc::chargeAlways);
      return ERROR_STATE;
    }
  }


  smsc->ReportDelivery(resp->cntx.inDlgId,*sms,true,Smsc::chargeOnSubmit);

  //
  // stored
  //
  ////

  onSubmitOk(t.msgId,*sms);


  if(!isDatagram && !isTransaction && needToSendResp) // Store&Forward mode
  {
    char buf[64];
    sprintf(buf,"%lld",t.msgId);
    SmscCommand response = SmscCommand::makeSubmitSmResp
                         (
                           buf,
                           dialogId,
                           Status::OK,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    response->dstNodeIdx=t.command->dstNodeIdx;
    response->sourceId=t.command->sourceId;
    try{
      src_proxy->putCommand(response);
    }catch(...)
    {
      warn2(smsLog, "SBM: failed to put response command SUBMIT_OK Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
        t.msgId,dialogId,
        sms->getOriginatingAddress().toString().c_str(),
        sms->getDestinationAddress().toString().c_str(),
        src_proxy->getSystemId(),
        sms->getDestinationSmeId()
      );
    }
  }

  if(!resp->cntx.generateDeliver)
  {
    smsc_log_debug(smsLog,"leave non merged Id=%lld in enroute state",t.msgId);
    return ENROUTE_STATE;
  }

  smsc_log_debug(smsLog,"Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(!isDatagram && !isTransaction && stime>now)
  {
    smsc->getScheduler()->AddScheduledSms(t.msgId,*sms,dest_proxy_index);
    sms->setLastResult(Status::DEFERREDDELIVERY);
    smsc->ReportDelivery(resp->cntx.inDlgId,*sms,false,Smsc::chargeOnDelivery);
    return ENROUTE_STATE;
  }

  //      Datagram  Transction .
  //      submit  submit response,   ...
  //         ,
  //  sms  setLastError()
  struct ResponseGuard{
    SMS *sms;
    SmeProxy* prx;
    StateMachine* sm;
    SMSId msgId;
    int dstNodeIndex;
    SmeSystemId smeSysId;
    /*ResponseGuard():sms(0),prx(0),sm(0){}
    ResponseGuard(const ResponseGuard& rg)
    {
      sms=rg.sms;
      prx=rg.prx;
      sm=rg.sm;
    }*/
    ResponseGuard(SMS* s,SmeProxy* p,StateMachine *st,SMSId id):sms(s),prx(p),sm(st),msgId(id){}
    ~ResponseGuard()
    {
      if(!sms)return;

      if(sms->lastResult!=Status::OK)
      {
        sm->onDeliveryFail(msgId,*sms);
      }

      bool sandf=(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0 ||
                 (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x3;

      if(!sandf && sms->lastResult!=Status::OK)
      {
        SmscCommand resp = SmscCommand::makeSubmitSmResp
            (
                "0",
                sms->dialogId,
                sms->lastResult,
                sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
            );
        resp->dstNodeIdx=dstNodeIndex;
        resp->sourceId=smeSysId;
        try{
          prx->putCommand(resp);
        }catch(...)
        {
          warn1(sm->smsLog,"SUBMIT: failed to put response command");
        }
      }
    }
  };

  ResponseGuard respguard(sms,src_proxy,this,t.msgId);
  respguard.dstNodeIndex=t.command->dstNodeIdx;
  respguard.smeSysId=t.command->sourceId;

  struct DeliveryReportGuard{
    Smsc* smsc;
    SMS* sms;
    INSmsChargeResponse::SubmitContext* cntx;
    bool final;
    bool active;
    DeliveryReportGuard():active(true){}
    ~DeliveryReportGuard()
    {
      if(active)
      {
        smsc->ReportDelivery(cntx->inDlgId,*sms,final,Smsc::chargeOnDelivery);
      }
    }
  };
  DeliveryReportGuard repGuard;
  repGuard.smsc=smsc;
  repGuard.sms=sms;
  repGuard.cntx=&resp->cntx;
  repGuard.final=isDatagram || isTransaction;

  if ( !dest_proxy )
  {
    sms->setLastResult(Status::SMENOTCONNECTED);
    info2(smsLog, "SBM: dest sme not connected Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      sms->getDestinationSmeId()
    );
    Descriptor d;
    sendNotifyReport(*sms,t.msgId,"destination unavailable");
    changeSmsStateToEnroute(*sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));
    return ENROUTE_STATE;
  }
  // create task
  uint32_t dialogId2;
  uint32_t uniqueId=dest_proxy->getUniqueId();
  try{
     dialogId2=dest_proxy->getNextSequenceNumber();
  }catch(...)
  {
    sms->setLastResult(Status::SMENOTCONNECTED);
    Descriptor d;
    changeSmsStateToEnroute(*sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));
    warn2(smsLog, "SBM: failed to get seq number Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      sms->getDestinationSmeId()
    );
    sendNotifyReport(*sms,t.msgId,"destination unavailable");
    return ENROUTE_STATE;
  }
  //Task task((uint32_t)c.dest_proxy_index,dialogId2);

  TaskGuard tg;
  tg.smsc=smsc;
  tg.dialogId=dialogId2;
  tg.uniqueId=uniqueId;

  bool taskCreated=false;
  try{
    Task task(uniqueId,dialogId2,isDatagram || isTransaction?new SMS(*sms):0);
    task.messageId=t.msgId;
    task.diverted=diverted;
    task.inDlgId=resp->cntx.inDlgId;
    if ( smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      taskCreated=true;
      tg.active=true;
    }
  }catch(...)
  {
  }

  if(!taskCreated)
  {
    sms->setLastResult(Status::SYSERR);
    Descriptor d;
    changeSmsStateToEnroute(*sms,t.msgId,d,Status::SYSERR,rescheduleSms(*sms));

    warn2(smsLog, "SBMDLV: failed to create task, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      sms->getDestinationSmeId()
    );
    sendNotifyReport(*sms,t.msgId,"system failure");
    return ENROUTE_STATE;
  }

  Address srcOriginal=sms->getOriginatingAddress();
  Address dstOriginal=sms->getDestinationAddress();

  bool deliveryOk=false;
  int  err=0;
  smsc::core::buffers::FixedLengthString<64> errstr;

  try{

    if( !isForwardTo )
    {
      // send delivery
      Address src;
      smsc_log_debug(smsLog,"SUBMIT: Id=%lld wantAlias=%s, hide=%s",t.msgId,dstSmeInfo.wantAlias?"true":"false",HideOptionToText(sms->getIntProperty(Tag::SMSC_HIDE)));
#ifdef SMSEXTRA
      if(sms->getIntProperty(Tag::SMPP_PRIVACYINDICATOR)==2)
      {
        Profile srcprof=smsc->getProfiler()->lookup(sms->getOriginatingAddress());
        if(srcprof.nick.length())
        {
          try{
            Address nick((uint8_t)srcprof.nick.length(),5,0,srcprof.nick.c_str());
            sms->setOriginatingAddress(nick);
          }catch(...)
          {
            warn2(smsLog,"Failed to construct nick from '%s' for abonent '%s'",srcprof.nick.c_str(),sms->getOriginatingAddress().toString().c_str());
          }
        }
      }else
#endif
      if(
          dstSmeInfo.wantAlias &&
          sms->getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
          routeHide &&
          smsc->AddressToAlias(sms->getOriginatingAddress(),src)
        )
      {
        sms->setOriginatingAddress(src);
      }
#ifdef SMSEXTRA
      if(!noDestChange)
      {
#endif
      sms->setDestinationAddress(dst);
#ifdef SMSEXTRA
      }
#endif

      // profile lookup performed before partitioning
      //profile=smsc->getProfiler()->lookup(dst);
      //
      if(!sms->hasBinProperty(Tag::SMSC_CONCATINFO) && sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && !transit)
      {
        using namespace smsc::profiler::ProfileCharsetOptions;
        if(
           (
             (sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)==Default ||
              sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)==Latin1
             )
             && sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2
           ) ||
           (
             (sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)&Latin1)!=Latin1 &&
             sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::LATIN1
           )
          )
        {
          try{
            transLiterateSms(sms,sms->getIntProperty(Tag::SMSC_DSTCODEPAGE));
            if(sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
            {
              int dc=sms->getIntProperty(Tag::SMSC_ORIGINAL_DC);
              int olddc=dc;
              if((dc&0xc0)==0 || (dc&0xf0)==0xf0) //groups 00xx and 1111
              {
                dc&=0xf3; //11110011 - clear 2-3 bits (set alphabet to default).

              }else if((dc&0xf0)==0xe0)
              {
                dc=0xd0 | (dc&0x0f);
              }
              sms->setIntProperty(Tag::SMSC_ORIGINAL_DC,dc);
              smsc_log_debug(smsLog,"SUBMIT: transliterate olddc(%x)->dc(%x)",olddc,dc);
            }
          }catch(exception& e)
          {
            smsc_log_warn(smsLog,"SUBMIT:Failed to transliterate: %s",e.what());
          }
        }
      }
    }

    if(sms->hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(!extractSmsPart(sms,0))
      {
        smsc_log_error(smsLog,"Id=%lld:failed to extract sms part, aborting.",t.msgId);
        err=Status::SYSERR;
        throw ExtractPartFailedException();
      };
      sms->setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
    }

    if(replyPath==smsc::router::ReplyPathForce)
    {
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(replyPath==smsc::router::ReplyPathSuppress)
    {
      sms->setIntProperty(Tag::SMPP_ESM_CLASS,sms->getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }

    smsc_log_debug(smsLog,"SBM: Id=%lld, esm_class=%x",t.msgId,sms->getIntProperty(Tag::SMPP_ESM_CLASS));

    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    unsigned bodyLen=0;
    delivery->get_sms()->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&bodyLen);
    int prio=priority/1000;
    if(prio<0)prio=0;
    if(prio>=32)prio=31;
    delivery->set_priority(prio);
    try{
      hrtime_t putCommandStart;

      if(perfLog->isInfoEnabled())
      {
        putCommandStart=gethrtime();
      }
      dest_proxy->putCommand(delivery);

      if (perfLog->isInfoEnabled())
      {
        hrtime_t putCommandTime=(gethrtime()-putCommandStart)/1000000;
        if(putCommandTime>20)
        {
          smsc_log_info(perfLog,"put command time=%lld",putCommandTime);
        }
      }
      deliveryOk=true;
    }catch(InvalidProxyCommandException& e)
    {
      err=Status::INVBNDSTS;
      //sendNotifyReport(*sms,t.msgId,"service rejected");
      errstr="invalid bind state";
    }
  }catch(ExtractPartFailedException& e)
  {
    errstr="Failed to extract sms part";
    err=Status::INVPARLEN;
  }catch(exception& e)
  {
    errstr=e.what();
    //sendNotifyReport(*sms,t.msgId,"system failure");
    err=Status::THROTTLED;
  }catch(...)
  {
    err=Status::THROTTLED;
    errstr="unknown";
    //sendNotifyReport(*sms,t.msgId,"system failure");
  }
  if(!deliveryOk)
  {
    warn2(smsLog, "SBMDLV: failed to put delivery command, seqnum=%d Id=%lld;seq=%d;oa=%s;da=%s;srcprx=%s;dstprx=%s - %s",
      dialogId2,
      t.msgId,dialogId,
      sms->getOriginatingAddress().toString().c_str(),
      sms->getDestinationAddress().toString().c_str(),
      src_proxy->getSystemId(),
      sms->getDestinationSmeId(),
      errstr.c_str()
    );
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->setLastResult(err);
    if(Status::isErrorPermanent(err))
      sendFailureReport(*sms,t.msgId,err,"system failure");
    else
      sendNotifyReport(*sms,t.msgId,"system failure");

    if(!isDatagram && !isTransaction)
    {
      try{
        Descriptor d;
        if(Status::isErrorPermanent(err))
        {
          store->changeSmsStateToUndeliverable(t.msgId,d,err);
          repGuard.final=true;
        }
        else
          changeSmsStateToEnroute(*sms,t.msgId,d,err,rescheduleSms(*sms));
      }catch(...)
      {
        smsc_log_warn(smsLog,"SUBMIT: failed to change state of Id=%lld to enroute/undeliverable",t.msgId);
      }
    }
    return Status::isErrorPermanent(err)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }

  tg.active=false;
  repGuard.active=false;

  sms->lastResult=Status::OK;
  info2(smsLog,"SBM: submit ok, seqnum=%d Id=%lld;seq=%d;%s;%s;srcprx=%s;dstprx=%s;valid=%lu",
    dialogId2,
    t.msgId,dialogId,
    AddrPair("oa",sms->getOriginatingAddress(),"ooa",srcOriginal).c_str(),
    AddrPair("da",dstOriginal,"dda",sms->getDestinationAddress()).c_str(),
    src_proxy->getSystemId(),
    sms->getDestinationSmeId(),
    sms->getValidTime()
  );
  return DELIVERING_STATE;
}



}//system
}//smsc
