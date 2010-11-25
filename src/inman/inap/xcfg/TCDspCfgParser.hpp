/* ************************************************************************** *
 * TCAP Dispatcher configuration parsers.
 * ************************************************************************** */
#ifndef __INMAN_TCAP_CFGS_PARSERS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_CFGS_PARSERS_HPP__

#ifdef EIN_HD
#include "inman/inap/xcfg/SS7HDCfgParser.hpp"
#else  /* EIN_HD */
#include "inman/inap/xcfg/SS7HRCfgParser.hpp"
#endif /* EIN_HD */


namespace smsc  {
namespace inman {
namespace inap {

#ifdef EIN_HD
typedef SS7HDCfgParser TCDspCfgParser;
#else  /* EIN_HD */
typedef SS7HRCfgParser TCDspCfgParser;
#endif /* EIN_HD */

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_CFGS_PARSERS_HPP__ */

