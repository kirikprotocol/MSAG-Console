/* ************************************************************************* *
 * USSMan global return codes registry initialization
 * ************************************************************************* */
#ifndef __SMSC_USSMAN_URCREGISTRY_INIT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_URCREGISTRY_INIT_HPP

#ifdef URCREGISTRY_GLOBAL_INIT
#include "inman/inap/SS7MsgErrors.hpp"
#include "inman/inap/TCAPErrors.hpp"
#include "inman/inap/TCDlgErrors.hpp"
#include "inman/comp/MapOpErrors.hpp"

#  define URCRegistryGlobalInit() _RCS_SS7_CP_GET(); \
_RCS_TCAPErrorsGET(); _RCS_TC_DialogGET(); _RCS_MapOpErrorsGET()

#else /* URCREGISTRY_GLOBAL_INIT */
#  define URCRegistryGlobalInit() 
#endif /* URCREGISTRY_GLOBAL_INIT */

#endif /* __SMSC_INMAN_URCREGISTRY_INIT_HPP */

