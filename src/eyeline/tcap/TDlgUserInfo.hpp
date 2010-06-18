/* ************************************************************************** *
 * TCAP dialogue user information.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_USRINFO_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_USRINFO_HPP

#include <list>
#include "eyeline/asn1/ASExternal.hpp"

namespace eyeline {
namespace tcap {

typedef std::list<const asn1::ASExternal*> TDlgUserInfoPtrList;
typedef std::list<asn1::ASExternal> TDlgUserInfoList;

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_USRINFO_HPP */

