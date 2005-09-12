static char const ident[] = "$Id$";
#include "comps.hpp"
#include <vector>
#include <RequestReportSMSEventArg.h>
namespace smsc {
namespace inman {
namespace comp{
using std::vector;

class InternalRequestReportSMSEventArg{
  public:
    SMSEventVector events;
};


RequestReportSMSEventArg::RequestReportSMSEventArg()
{
  internal = new InternalRequestReportSMSEventArg();
}
RequestReportSMSEventArg::~RequestReportSMSEventArg()
{
  delete(internal);
}
int RequestReportSMSEventArg::decode(const vector<unsigned char>& buf)
{
  RequestReportSMSEventArg_t *req = 0;
  asn_dec_rval_t rval;

  rval = ber_decode(0, &asn_DEF_RequestReportSMSEventArg,(void **)&req, &buf[0], buf.size());
  if (req)
  {
    const asn_anonymous_set_ *list = _A_CSEQUENCE_FROM_VOID(&req->sMSEvents);

    asn_TYPE_member_t *elm = td->elements;
    internal->events.size(list->count);
    for(int i = 0; i < list->count; i++) {
      const sMSEvents *elem = list->array[i];
      if(!elem)
      {
        internal->events[i].event = EventTypeSMS_t_NONE;
        continue;
      }
      internal->events[i].event = elem->eventTypeSMS;
      internal->events[i].monitorType = elem->monitorMode;

    }
    asn_DEF_RequestReportSMSEventArg.free_struct(&asn_DEF_RequestReportSMSEventArg,req, 0);
  }
  return rval.code || !structure;
}
int RequestReportSMSEventArg::encode(vector<unsigned char>& buf)
{
  return -1; //not implemented yet
}

}//namespace comps
}//namespace inman
}//namespace smsc
