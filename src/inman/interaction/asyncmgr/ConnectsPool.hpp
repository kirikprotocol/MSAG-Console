/* ************************************************************************* *
 * Definition of asynchronous Connect objects pool and guard.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ASYNCMGR_CONNECTS_POOL
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ASYNCMGR_CONNECTS_POOL

#include "inman/common/GrdObjPool_T.hpp"
#include "inman/interaction/asynconn/Connect.hpp"

namespace smsc {
namespace inman {
namespace interaction {

typedef smsc::util::GrdObjPool_T<Connect, unsigned> ConnectsPool;
typedef ConnectsPool::ObjRef  ConnectGuard;

} //interaction
} //inman
} //smsc
#endif /* __SMSC_INMAN_ASYNCMGR_CONNECTS_POOL */

