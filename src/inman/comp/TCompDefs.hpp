/* ************************************************************************* *
 * Base class for 'asn1-encodable' ROS primitives (ARGUMENT,RESULT,ERROR),
 * which are transported by TCAP in ComponentPortion.
 * ************************************************************************* */
#ifndef __UMTS_TCAP_COMP_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UMTS_TCAP_COMP_DEFS_HPP

#include <vector>

#include "util/Exception.hpp"

namespace umts {
namespace tcap {

//Note: Component encoding size type is uint16_t, because of encoding
//      of a Component cann't exceed TCAP message length.
class TComponentAC {
public:
  virtual ~TComponentAC()
  { }

  // ------------------------------------------
  // -- TComponentAC interface methods:
  // ------------------------------------------
  virtual void encode(std::vector<unsigned char> & out_buf) const
    throw(smsc::util::CustomException)
  {
    throw smsc::util::CustomException(-1, "ASN.1 encoding is not implemented");
  }
  //
  virtual void decode(const uint8_t * in_buf, uint16_t enc_len)
    throw(smsc::util::CustomException)
  {
    throw smsc::util::CustomException(-1, "ASN.1 decoding is not implemented");
  }
  //Implement this method if component is a primitive of service that supports
  //ROS RESULT segmentation while transferring by TCAP (for ex. 3GPP TS 29.002 Annex C).
  virtual void mergeSegment(TComponentAC * use_segm)
    throw(smsc::util::CustomException)
  {
    throw smsc::util::CustomException(-2, "TC-Result-NL segmentation is not supported");
  }
};

} //tcap
} //umts

#endif /* __UMTS_TCAP_COMP_DEFS_HPP */
