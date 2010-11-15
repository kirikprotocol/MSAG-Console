/* ************************************************************************** *
 * IAProvider utilizing MAP service ATSI.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPRVD_ATSI_LOADER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPRVD_ATSI_LOADER_HPP

#include "inman/services/ICSrvDefs.hpp"

namespace smsc {
namespace inman {

//Generic ICService loading function
extern ICSProducerAC * ICSLoaderIAPrvdATSI(void);

} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_ATSI_LOADER_HPP */

