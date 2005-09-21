static char const ident[] = "$Id$";
#include <vector>
#include <assert.h>
#include "RequestReportSMSEventArg.h"
#include "comps.hpp"
#include "compsutl.hpp"


namespace smsc {
namespace inman {
namespace comp{
using std::vector;

class InternalRequestReportSMSEventArg
{
  public:
    RequestReportSMSEventArg::SMSEventVector events;
};


RequestReportSMSEventArg::RequestReportSMSEventArg()
{
  comp = new InternalRequestReportSMSEventArg();
  compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.RequestReportSMSEventArg");
}

RequestReportSMSEventArg::~RequestReportSMSEventArg()
{
  delete(comp);
}

void RequestReportSMSEventArg::decode(const vector<unsigned char>& buf)
{
  RequestReportSMSEventArg_t *req = 0;
  asn_dec_rval_t rval;

  rval = ber_decode(0, &asn_DEF_RequestReportSMSEventArg,(void **)&req, &buf[0], buf.size());
  INMAN_LOG_DEC(rval, asn_DEF_RequestReportSMSEventArg);

  assert( req );

  const asn_anonymous_sequence_ * list = _A_CSEQUENCE_FROM_VOID(&req->sMSEvents);
    
  for(int i = 0; i < list->count; i++) 
  {
	  ::SMSEvent *elem = static_cast< ::SMSEvent* >(list->array[i]);
      if(!elem)
      {
        //internal->events[i].event = EventTypeSMS_t_NONE;
        continue;
      }

      RequestReportSMSEventArg::SMSEvent smsEvent;

      smsEvent.event 	   = 
      	static_cast< EventTypeSMS_e >( elem->eventTypeSMS );

      smsEvent.monitorType = 
      	static_cast< MonitorMode_e >( elem->monitorMode );

      comp->events.push_back( smsEvent );
  }

  smsc_log_component(compLogger, &asn_DEF_RequestReportSMSEventArg, req);
  asn_DEF_RequestReportSMSEventArg.free_struct(&asn_DEF_RequestReportSMSEventArg,req, 0);
}

//void RequestReportSMSEventArg::encode(vector<unsigned char>& buf)
//{ throw EncodeError("Not implemented"); }

const RequestReportSMSEventArg::SMSEventVector& RequestReportSMSEventArg::getSMSEvents()
{
    return comp->events;
}

}//namespace comps
}//namespace inman
}//namespace smsc

