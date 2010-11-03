#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************* *
 * INMan global return codes registry initialization
 * ************************************************************************* */
#include "inman/INManErrors.hpp"

namespace smsc {
namespace inman {

_RCS_INManErrorsINIT();

} //inman
} //smsc

#ifdef URCREGISTRY_GLOBAL_INIT
#include "inman/inap/SS7MsgErrors.hpp"
#include "inman/inap/TCAPErrors.hpp"
#include "inman/inap/TCDlgErrors.hpp"
namespace smsc {
namespace inman {
namespace inap {
_RCS_SS7_CP_INIT();
_RCS_TCAPErrorsINIT();
_RCS_TC_DialogINIT();
} //inap
} //inman
} //smsc

#include "inman/comp/MapOpErrors.hpp"
#include "inman/comp/CapOpErrors.hpp"
#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
namespace smsc {
namespace inman {
namespace comp {
_RCS_MapOpErrorsINIT();
_RCS_CapOpErrorsINIT();
_RCS_MOSM_RPCauseINIT();
} //comp
} //inman
} //smsc

#include "inman/abprov/IAPErrors.hpp"
namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders
_RCS_IAPQStatusINIT();
} //iaprvd
} //inman
} //smsc
#endif /* URCREGISTRY_GLOBAL_INIT */

