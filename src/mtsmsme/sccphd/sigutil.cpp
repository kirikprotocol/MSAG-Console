static char const ident[] = "$Id$";
#include "mtsmsme/sccphd/sigutil.hpp"
namespace smsc{namespace mtsmsme{namespace processor{namespace util{

const char* getReturnOptionDescription(UCHAR_T opt) {
  switch(opt) {
    case EINSS7_SCCP_RET_OPT_OFF: return "\"Discard message if fail\"";
    case EINSS7_SCCP_RET_OPT_ON: return "\"Return message if fail\"";
    default: return "\"UNKNOWN return option\"";
  }
}
const char* getSequenceControlDescription(UCHAR_T ctrl) {
  switch(ctrl) {
    case EINSS7_SCCP_SEQ_CTRL_OFF: return "\"PROTO CLASS=0\"";
    case EINSS7_SCCP_SEQ_CTRL_ON: return "\"PROTO CLASS=1\"";
    default: return "\"UNKNOWN protocol class\"";
  }
}
const char* getSccpBindStatusDescription(UCHAR_T result)
{
  switch (result)
  {
    case EINSS7_SCCP_NB_SUCCESS: return "\"Bind success\"";
    case EINSS7_SCCP_SSN_ALREADY_IN_USE: return "\"Subsystem number in use\"";
    case EINSS7_SCCP_PROTOCOL_ERROR: return "\"Protocol error\"";
    case EINSS7_SCCP_NB_RES_UNAVAIL: return "\"Resources unavailable\"";
    case EINSS7_SCCP_SSN_NOT_ALLOWED: return "\"Invalid subsystem number\"";
    case EINSS7_SCCP_SCCP_NOT_READY: return "\"SCCP Not available\"";
    case 6: return "\"Invalid NodeID\"";
    default: return "UNKNOWN STATUS ERROR";
  }
}

}/* namespace util */}/* namespace processor */}/* namespace mtsms */}/* namespace smsc */
