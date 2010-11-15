/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) types, interfaces and
 * helpers definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_SCF_REGISTRY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_SCF_REGISTRY_HPP

#include "inman/common/ObjRegistryT.hpp"
#include "inman/services/iapmgr/InScfCfg.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

using smsc::util::TonNpiAddressString;

typedef smsc::util::POBJRegistry_T<INScfIdent_t, INScfCFG>  SCFRegistry;
typedef std::map<TonNpiAddressString, const INScfCFG *> INScfsMAP;

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_SCF_REGISTRY_HPP */

