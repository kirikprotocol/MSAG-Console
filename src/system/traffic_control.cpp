#include "util/debug.h"
#include "system/traffic_control.hpp"
#include "system/smsc.hpp"
#include "store/StoreManager.h"

namespace smsc{
namespace system{

using namespace smsc::profiler;


bool TrafficControl::processCommand(SmscCommand& cmd)
{
  MutexGuard g(mtx);
  bool rv=false;
  switch(cmd->get_commandId())
  {
    case SUBMIT:
    case FORWARD:
    {

      /*
      if(!isReceipt)
      {
        double tcnt=totalCounter.Get()+1;
        __debug2__(log,"TC: totcalCount=%lf",tcnt);
        if(tcnt/cfg.shapeTimeFrame>cfg.maxSmsPerSecond)
        {
          __info2__(log,"TC: deny - SHAPING LIMIT!!! speed=%lf",tcnt/cfg.shapeTimeFrame);
          return false;
        }
      }
      */

      Address dst;
      SMS *sms;
      SMS _sms;

      if(cmd->get_commandId()==SUBMIT)
      {
        sms=cmd->get_sms();
      }else
      {
        try{
          cfg.store->retriveSms(cmd->get_forwardMsgId(),_sms);
        }catch(...)
        {
          __warn2__(log,"TC: failed to retrieve sms with id=%lld",cmd->get_forwardMsgId());
          break;
        }
        sms=&_sms;
      }
      bool isReceipt=sms->hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID);


      if(!cfg.smsc->AliasToAddress(sms->getDestinationAddress(),dst))
      {
        dst=sms->getDestinationAddress();
      }
      SmeProxy *src_proxy=cmd.getProxy();

      smsc::router::RouteInfo ri;
      SmeIndex dstIdx;
      SmeProxy *dest_proxy;
      bool has_route = cfg.smsc->routeSms(sms->getOriginatingAddress(),
                                      dst,dstIdx,dest_proxy,&ri);
      if(!has_route)return true;

      if(!isReceipt && cmd->get_commandId()==SUBMIT)
      {
        SmeInfo si=cfg.smsc->getSmeInfo(dstIdx);

        __trace2__("smeschedcount for %s = %d",si.systemId.c_str(),cfg.smsc->GetSmeScheduleCount(dstIdx,0));
        if(si.schedlimit!=0 && cfg.smsc->GetSmeScheduleCount(dstIdx,0)>=si.schedlimit)
        {
          __info__(log,"schedlimit");
          break;
        }
      }

      if(cmd->get_commandId()==SUBMIT &&
         !sms->hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
         !ri.suppressDeliveryReports &&
         (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&3)!=SMSC_TRANSACTION_MSG_MODE &&
         (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&3)!=SMSC_TRANSACTION_MSG_MODE)
      {
        smsc::profiler::Profile profile=cfg.smsc->getProfiler()->lookup(sms->getOriginatingAddress());

        bool regdel=(sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
                    (sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==2 ||
                     sms->getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST);

        if(regdel || profile.reportoptions!=ProfileReportOptions::ReportNone)
        {
          SmeIndex idx=src_proxy->getSmeIndex();
          SmeInfo si=cfg.smsc->getSmeInfo(idx);
          __trace2__("smeschedcount(receipt) for %s=%d",src_proxy->getSystemId(),cfg.smsc->GetSmeScheduleCount(idx,0));
          if(si.schedlimit!=0 && cfg.smsc->GetSmeScheduleCount(idx,0)>=si.schedlimit)
          {
            __info__(log,"receipt schedlimit");
            break;
          }
          IntTimeSlotCounter *dsrccnt=getTSC(deliverCnt,idx);
          IntTimeSlotCounter *rsrccnt=getTSC(responseCnt,idx);
          int deliveryCount=dsrccnt->Get();
          int responseCount=rsrccnt->Get();

          if(deliveryCount-responseCount>cfg.protectThreshold)
          {
            __info2__(log,"TC: deny - receipt protect threshold limit for %s: %d-%d",
              src_proxy->getSystemId(),deliveryCount,responseCount);
            break;
          }
          /*
          int scount=cfg.smsc->GetSmeScheduleCount(idx,lookAhead);
          double speed=(double)responseCount*cfg.lookAheadTime/cfg.protectTimeFrame;

          if(speed>1 && speed<scount)
          {
            __info2__(log,"TC: deny - receipt protect schedule limit for %s: %lf - %d",src_proxy->getSystemId(),speed,scount);
            break;
          }
          */
          dsrccnt->Inc();
          __debug2__(log,"TC: receipt inc for %s",src_proxy->getSystemId());
        }
      }



      if(!dest_proxy)return true;


      //time_t lookAhead=time(NULL)+cfg.lookAheadTime;

      IntTimeSlotCounter *dcnt=getTSC(deliverCnt,dstIdx);
      IntTimeSlotCounter *rcnt=getTSC(responseCnt,dstIdx);

      int deliveryCount=dcnt->Get();
      int responseCount=rcnt->Get();

      if(deliveryCount-responseCount>cfg.protectThreshold)
      {
        __info2__(log,"TC: deny - protect threshold limit for %s: %d/%d",dest_proxy->getSystemId(),responseCount,deliveryCount);
        break;
      }

      /*
      if(cmd->get_commandId()==SUBMIT)
      {
        int scount=cfg.smsc->GetSmeScheduleCount(dstIdx,lookAhead);

        double speed=(double)responseCount*cfg.lookAheadTime/cfg.protectTimeFrame;

        if(speed>1 && speed<scount)
        {
          __info2__(log,"TC: deny - protect schedule limit for %s: %lf - %d",dest_proxy->getSystemId(),speed,scount);
          break;
        }
      }
      */


      if(!sms->hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        __debug2__(log,"TC: inc for %s",dest_proxy->getSystemId());
        dcnt->Inc();
      }

      //totalCounter.Inc();
      //__debug2__(log,"TC: inc total count(%d)",totalCounter.Get());

      rv=true;

    }break;
    case DELIVERY_RESP:
      if(GET_STATUS_TYPE(cmd->get_resp()->get_status())==CMD_OK ||
         GET_STATUS_TYPE(cmd->get_resp()->get_status())==CMD_ERR_PERM ||
         GET_STATUS_TYPE(cmd->get_resp()->get_status())==CMD_ERR_FATAL)
      {
        __debug2__(log,"TC: delresp for %s",cmd.getProxy()->getSystemId());
        getTSC(responseCnt,cmd.getProxy()->getSmeIndex())->Inc();
      } // there is no break. там и задумано
    default:
      return true;
  }
  if(!rv)
  {
    totalCounter.Inc(-1);
  }
  return rv;
}


}//system
}//smsc
