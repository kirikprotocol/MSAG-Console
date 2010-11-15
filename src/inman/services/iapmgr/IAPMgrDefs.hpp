/* ************************************************************************** *
 * IAPManager interface definitions.
 * ************************************************************************** */
#ifndef __INMAN_IAPMANAGER_DEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMANAGER_DEFS_HPP__

#include "inman/services/iapmgr/AbonentPolicy.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

class IAPManagerITF {
protected:
  virtual ~IAPManagerITF() //forbid interface destruction
  { }

public:

  virtual const AbonentPolicy *
    getPolicy(const AbonentPolicyName_t & nm_pol) const = 0;

  virtual const IAPRule *
    getPolicy(const TonNpiAddress & subscr_addr) const = 0;
};


} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_IAPMANAGER_DEFS_HPP__ */

