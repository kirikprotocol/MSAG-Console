#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TCServiceAllocator.hpp"
#include "eyeline/tcap/provd/TCService.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TCProviderAC * allocTCService(void) /*throw()*/
{
  return new TCService();
}

}}}

