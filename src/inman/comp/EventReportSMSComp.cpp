static char const ident[] = "$Id$";
#include "comps.hpp"
#include <vector>
#include <EventReportSMSArg.h>

namespace smsc {
namespace inman {
namespace comp{
using std::vector;

extern "C" static int print2vec(const void *buffer, size_t size, void *app_key);

EventReportSMSArg::EventReportSMSArg(EventTypeSMS_e eType, MessageType_e mType)
	: eventType( eType )
	, messageType( mType )
{
}

EventReportSMSArg::~EventReportSMSArg()
{
}

void EventReportSMSArg::decode(const vector<unsigned char>& buf)
{
  throw DecodeError("Not implemented");
}

void EventReportSMSArg::encode(vector<unsigned char>& buf)
{
  EventReportSMSArg_t eventReport;
  memset( &eventReport, 0x00, sizeof( EventReportSMSArg_t ) );
  eventReport.eventTypeSMS = eventType;
  
  MiscCallInfo_t mcs;
  memset( &mcs, 0x00, sizeof( MiscCallInfo_t ) );
  mcs.messageType = messageType;

  eventReport.miscCallInfo = &mcs;

  asn_enc_rval_t er = der_encode(&asn_DEF_EventReportSMSArg, &eventReport, print2vec, &buf);

  if(er.encoded == -1) 
  {
    throw EncodeError( format( "Cannot encode type %s", er.failed_type->name ) );
  }
}

static int print2vec(const void *buffer, size_t size, void *app_key) 
{
  std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
  unsigned char *buf = (unsigned char *)buffer;
  stream->insert(stream->end(),buf, buf + size);
  return 0;
}

}//namespace comps
}//namespace inman
}//namespace smsc
