/* ************************************************************************** *
 * TCAP UNIDialogue PDUs definitions according to
 * itu-t recommendation q 773 modules(2) unidialoguePDUs(3) version1(1).
 * ************************************************************************** */
#ifndef __TC_DIALOGUE_UNI_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_DIALOGUE_UNI_DEFS_HPP

#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/tcap/proto/ProtocolVersion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

extern const asn1::EncodedOID _ac_tcap_uniDialogue_as;

struct TCPduAUDT {
  ProtocolVersion         _protoVer;
  asn1::EncodedOID        _acId;
  tcap::TDlgUserInfoList  _usrInfo;    //optional

  explicit TCPduAUDT() : _protoVer(_dfltProtocolVersion)
  { }
  ~TCPduAUDT()
  { }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_DIALOGUE_UNI_DEFS_HPP */

