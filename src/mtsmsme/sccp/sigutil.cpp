static char const ident[] = "$Id$";
#include "mtsmsme/sccp/sigutil.hpp"
namespace smsc{namespace mtsmsme{namespace processor{namespace util{

const char* getReturnOptionDescription(uint8_t opt) {
  switch(opt) {
    case EINSS7_I96SCCP_RET_OPT_OFF: return "\"Discard message if fail\"";
    case EINSS7_I96SCCP_RET_OPT_ON: return "\"Return message if fail\"";
    default: return "\"UNKNOWN return option\"";
  }
}
const char* getSequenceControlDescription(uint8_t ctrl) {
  switch(ctrl) {
    case EINSS7_I96SCCP_SEQ_CTRL_OFF: return "\"PROTO CLASS=0\"";
    case EINSS7_I96SCCP_SEQ_CTRL_ON: return "\"PROTO CLASS=1\"";
    default: return "\"UNKNOWN protocol class\"";
  }
}
const char* getSccpBindStatusDescription(uint8_t result)
{
  switch (result)
  {
    case EINSS7_I96SCCP_NB_SUCCESS: return "\"Bind success\"";
    case EINSS7_I96SCCP_SSN_ALREADY_IN_USE: return "\"Subsystem number in use\"";
    case EINSS7_I96SCCP_PROTOCOL_ERROR: return "\"Protocol error\"";
    case EINSS7_I96SCCP_NB_RES_UNAVAIL: return "\"Resources unavailable\"";
    case EINSS7_I96SCCP_SSN_NOT_ALLOWED: return "\"Invalid subsystem number\"";
    case EINSS7_I96SCCP_SCCP_NOT_READY: return "\"SCCP Not available\"";
    default: return "UNKNOWN STATUS ERROR";
  }
}
}/* namespace util */}/* namespace processor */}/* namespace mtsms */}/* namespace smsc */
