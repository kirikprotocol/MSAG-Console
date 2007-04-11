#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_ACREPO_HPP__
#define __SMSC_MTSMSME_PROCESSOR_ACREPO_HPP__

#include "Message.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

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

extern BIT_STRING_t tcapversion;
extern ASN__PRIMITIVE_TYPE_t pduoid;

void SuperTestFunction();

/* namespace processor */ } /* namespace mtsmsme */ } /* namespace smsc */ }

#endif /* __SMSC_MTSMSME_PROCESSOR_ACREPO_HPP__ */
