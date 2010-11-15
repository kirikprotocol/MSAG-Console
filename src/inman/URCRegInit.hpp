/* ************************************************************************* *
 * INMan Unified
 * ************************************************************************* */
#ifndef __SMSC_INMAN_URCREGISTRY_INIT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_URCREGISTRY_INIT_HPP

#include "inman/INManErrors.hpp"

#ifdef URCREGISTRY_GLOBAL_INIT
#include "inman/inap/SS7MsgErrors.hpp"
#include "inman/inap/TCAPErrors.hpp"
#include "inman/inap/TCDlgErrors.hpp"
#include "inman/comp/MapOpErrors.hpp"
#include "inman/comp/CapOpErrors.hpp"
#include "inman/comp/cap_sms/MOSM_RPCauses.hpp"
#include "inman/abprov/IAPErrors.hpp"

#  define URCRegistryGlobalInit() _RCS_INManErrorsGET(); \
_RCS_SS7_CP_GET(); _RCS_TCAPErrorsGET(); _RCS_TC_DialogGET(); \
_RCS_MapOpErrorsGET(); _RCS_CapOpErrorsGET(); _RCS_MOSM_RPCauseGET(); \
_RCS_IAPQStatusGET()

#else /* URCREGISTRY_GLOBAL_INIT */
#  define URCRegistryGlobalInit() _RCS_INManErrorsGET()
#endif /* URCREGISTRY_GLOBAL_INIT */

#endif /* __SMSC_INMAN_URCREGISTRY_INIT_HPP */

