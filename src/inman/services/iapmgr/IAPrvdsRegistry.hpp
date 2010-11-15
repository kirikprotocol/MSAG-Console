/* ************************************************************************** *
 * IAPManager: IAProviders registry.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_IAPROVIDERS_REG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_IAPROVIDERS_REG_HPP

#include "inman/common/ObjRegistryT.hpp"
#include "inman/services/iapmgr/IAProviderInfo.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

typedef smsc::util::POBJRegistry_T<IAProviderName_t, IAProviderInfo>  IAPrvdsRegistry;

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_IAPROVIDERS_REG_HPP */

