/* ************************************************************************* *
 * Definition of asynchronous Connect objects pool and guard.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ASYNCMGR_CONNECTS_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYNCMGR_CONNECTS_POOL

#include "core/buffers/IntrusivePoolT.hpp"
#include "inman/interaction/asynconn/Connect.hpp"

namespace smsc {
namespace inman {
namespace interaction {

typedef smsc::core::buffers::IntrusivePoolOf_T<Connect, unsigned, true> ConnectsPool;
typedef ConnectsPool::ObjRef  ConnectGuard;

} //interaction
} //inman
} //smsc
#endif /* __SMSC_INMAN_ASYNCMGR_CONNECTS_POOL */

