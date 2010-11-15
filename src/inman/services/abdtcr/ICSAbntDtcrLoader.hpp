/* ************************************************************************** *
 * Abonent detector service loader.
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNTDETECTOR_LOADER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_ABNTDETECTOR_LOADER_HPP

#include "inman/services/ICSrvDefs.hpp"

namespace smsc {
namespace inman {

//Generic ICService loading function
extern ICSProducerAC * ICSLoaderAbntDetector(void);

} //inman
} //smsc
#endif /* __INMAN_ICS_ABNTDETECTOR_LOADER_HPP */

