/* ************************************************************************** *
 * Transaction Capabilities Provider: service allocator.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_SERVICE_ALLC_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_SERVICE_ALLC_HPP

#include "eyeline/tcap/TCProviderIface.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

extern TCProviderAC * allocTCService(void) /*throw()*/;

} //provd
} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_SERVICE_ALLC_HPP */

