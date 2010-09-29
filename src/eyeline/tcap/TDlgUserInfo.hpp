/* ************************************************************************** *
 * TCAP dialogue user information.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_USRINFO_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
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

