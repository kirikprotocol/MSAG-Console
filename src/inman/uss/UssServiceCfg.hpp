#pragma ident "$Id$"
/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __SMSC_INMAN_USS_USSSERVICE_CFG_HPP__
# define __SMSC_INMAN_USS_USSSERVICE_CFG_HPP__

#include "inman/interaction/server.hpp"
#include "inman/inap/TCDspDefs.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

struct UssService_CFG {
  smsc::inman::interaction::ServSocketCFG sock;
  smsc::inman::inap::TCDsp_CFG   ss7;
  smsc::inman::inap::TCAPUsr_CFG tcUsr;
  smsc::inman::inap::TCAPDispatcherITF * tcDisp;

  UssService_CFG() : tcDisp(0)
  { }
};

} //uss
} //inman
} //smsc

#endif /* __SMSC_INMAN_USS_USSSERVICE_CFG_HPP__ */

