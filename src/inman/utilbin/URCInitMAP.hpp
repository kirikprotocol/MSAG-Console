/* ************************************************************************* *
 * Global return codes registry initialization for MAP services.
 * ************************************************************************* */
#ifndef __INMAN_URCREGISTRY_INIT_MAP_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_URCREGISTRY_INIT_MAP_HPP

#ifdef URCREGISTRY_GLOBAL_INIT
#include "inman/inap/SS7MsgErrors.hpp"
#include "inman/inap/TCAPErrors.hpp"
#include "inman/inap/TCDlgErrors.hpp"
#include "inman/comp/MapOpErrors.hpp"

#  define URCRegistryInit4MAP() _RCS_SS7_CP_GET(); \
_RCS_TCAPErrorsGET(); _RCS_TC_DialogGET(); _RCS_MapOpErrorsGET()

#else /* URCREGISTRY_GLOBAL_INIT */
#  define URCRegistryInit4MAP() /**/
#endif /* URCREGISTRY_GLOBAL_INIT */

#endif /* __INMAN_URCREGISTRY_INIT_MAP_HPP */

