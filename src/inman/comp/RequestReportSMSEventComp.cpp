static char const ident[] = "$Id$";
#include "comps.hpp"
#include <vector>
#include <assert.h>
#include <RequestReportSMSEventArg.h>
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
  internal = new InternalRequestReportSMSEventArg();
}

RequestReportSMSEventArg::~RequestReportSMSEventArg()
{
  delete(internal);
}

void RequestReportSMSEventArg::decode(const vector<unsigned char>& buf)
{
  RequestReportSMSEventArg_t *req = 0;
  asn_dec_rval_t rval;

  rval = ber_decode(0, &asn_DEF_RequestReportSMSEventArg,(void **)&req, &buf[0], buf.size());

  if( rval.code ) 
  {
    throw DecodeError( 
    	format( "Cannot decode %s: (error 0x%X on byte %d)",
    				"RequestReportSMSEventArg", rval.code, rval.consumed ) );
  }

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

      internal->events.push_back( smsEvent );
  }

  asn_DEF_RequestReportSMSEventArg.free_struct(&asn_DEF_RequestReportSMSEventArg,req, 0);
}

void RequestReportSMSEventArg::encode(vector<unsigned char>& buf)
{
  	throw EncodeError("Not implemented");
}

const RequestReportSMSEventArg::SMSEventVector& RequestReportSMSEventArg::getSMSEvents()
{
	return internal->events;
}

}//namespace comps
}//namespace inman
}//namespace smsc
