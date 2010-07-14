/* ************************************************************************** *
 * Various mobile addresses conversion utilities.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_ADDRESS_UTILS_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_ADDRESS_UTILS_HPP

#include "eyeline/sccp/SCCPAddress.hpp"
#include "eyeline/map/MobileAddress.hpp"
#include "eyeline/map/ISDNAddress.hpp"

namespace eyeline {
namespace map {

using eyeline::sccp::SCCPAddress;

//Attempts to convert SCCPAddress to MobileAddress.
//Returns true on success.
static bool convertSCCPAdr2Mobile(const SCCPAddress & sccp_adr, MobileAddress & map_adr);
//Attempts to convert ISDNAddress to MobileAddress.
//Returns true on success.
static bool convertSCCPAdr2ISDN(const SCCPAddress & sccp_adr, ISDNAddress & isdn_adr);

} //map
} //eyeline

#endif /* __EYELINE_MAP_ADDRESS_UTILS_HPP */

