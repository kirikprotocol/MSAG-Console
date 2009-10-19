/* ************************************************************************** *
 * SMS/USSD messages billing service loader.
 * ************************************************************************** */
#ifndef __INMAN_ICS_SMBILLING_LOADER_HPP
#ident "@(#)$Id$"
#define __INMAN_ICS_SMBILLING_LOADER_HPP

#include "inman/services/ICSrvDefs.hpp"

namespace smsc {
namespace inman {

//Generic ICService loading function
extern ICSProducerAC * ICSLoaderSmBilling(void);

} //inman
} //smsc
#endif /* __INMAN_ICS_SMBILLING_LOADER_HPP */

