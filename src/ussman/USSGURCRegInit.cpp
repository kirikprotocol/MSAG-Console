/* ************************************************************************* *
 * USS Gateway global return codes registry initialization.
 * ************************************************************************* */
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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
namespace smsc {
namespace inman {
namespace comp {
_RCS_MapOpErrorsINIT();
} //comp
} //inman
} //smsc

#endif /* URCREGISTRY_GLOBAL_INIT */

