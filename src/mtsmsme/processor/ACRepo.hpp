#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_ACREPO_HPP__
#define __SMSC_MTSMSME_PROCESSOR_ACREPO_HPP__

//#include "mtsmsme/processor/TSM.hpp"
#include <vector>
#include <string>
//#include "TCMessage.h"
#include "BIT_STRING.h"
#include "asn_codecs_prim.h"

namespace smsc{namespace mtsmsme{namespace processor{
using std::vector;
using std::string;

struct TrId{
  uint8_t buf[4];
  int size;
  bool operator==(const TrId& obj)const;
  string toString();
};
struct AC {
  vector<unsigned long> arcs;
  AC();
  void init(unsigned long *_arcs, int _size);
  AC(unsigned long *_arcs, int _size);
  bool operator==(const AC& obj)const;
  bool operator!=(const AC& obj)const;
  string toString();
};
extern AC null_ac;
extern AC sm_mt_relay_v1;
extern AC sm_mt_relay_v2;
extern AC sm_mt_relay_v3;
extern AC net_loc_upd_v1;
extern AC net_loc_upd_v2;
extern AC net_loc_upd_v3;
extern AC net_loc_cancel_v1;
extern AC net_loc_cancel_v2;
extern AC net_loc_cancel_v3;
extern AC shortMsgGatewayContext_v1;
extern AC shortMsgGatewayContext_v2;
extern AC shortMsgGatewayContext_v3;
extern AC shortMsgMoRelayContext_v2;
extern AC roamingNumberEnquiryContext_v1;
extern AC roamingNumberEnquiryContext_v2;
extern AC roamingNumberEnquiryContext_v3;
extern AC subscriberDataMngtContext_v1;
extern AC subscriberDataMngtContext_v2;
extern AC subscriberDataMngtContext_v3;
extern AC locationInfoRetrievalContext_v3;
extern BIT_STRING_t tcapversion;
extern ASN__PRIMITIVE_TYPE_t pduoid;

void SuperTestFunction();
class TSM;
class TCO;
extern TSM* createIncomingTSM(TrId ltrid,AC& ac,TCO* tco);
extern TSM* createOutgoingTSM(TrId ltrid,AC& appcntx,TCO* tco);
extern bool isIncomingContextSupported(AC& appcntx);
extern bool isMapV1ContextSupported(AC& appcntx);

/* namespace processor */ } /* namespace mtsmsme */ } /* namespace smsc */ }

#endif /* __SMSC_MTSMSME_PROCESSOR_ACREPO_HPP__ */
