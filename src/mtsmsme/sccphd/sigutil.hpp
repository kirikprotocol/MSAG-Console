#ident "$Id$"
#ifndef __SMSC_MTSMS_SIGUTIL_HPP__
#define __SMSC_MTSMS_SIGUTIL_HPP__
#include <string>
#include <inttypes.h>
#include "ss7cp.h"
#include "sccp_api.h"

namespace smsc{namespace mtsmsme{namespace processor{namespace util{

extern const char* getReturnOptionDescription(uint8_t opt);
extern const char* getSequenceControlDescription(uint8_t ctrl);
extern const char* getSccpBindStatusDescription(uint8_t result);

}/* namespace util */}/* namespace processor */}/* namespace mtsmsme */}/*namespace smsc */
#endif /* __SMSC_MTSMS_SIGUTIL_HPP__ */
