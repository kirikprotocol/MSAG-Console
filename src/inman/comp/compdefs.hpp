/* ************************************************************************* *
 * ROS Operations Component interface definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_COMP_DEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_COMP_DEFS_HPP__

#include "inman/comp/TCompDefs.hpp"

namespace smsc {
namespace inman {
namespace comp {

//using smsc::util::CustomException;

typedef umts::tcap::TComponentAC  Component;
typedef umts::tcap::TComponentAC  MAPComponent;

class ASN1EncodeError : public smsc::util::CustomException {
public:
  ASN1EncodeError(const char * def_name, const char * failed_type,
                  const char * comp_id = NULL)
    : smsc::util::CustomException("%s%sASN1 Encoding of %s failed at %s",
                                  comp_id ? comp_id : "", comp_id ? ": " : "",
                                  def_name, failed_type)
  {
    setExcId("ASN1EncodeError");
  }
};

class ASN1DecodeError : public smsc::util::CustomException {
public:
  ASN1DecodeError(const char * def_name, int er_code, size_t pos,
                  const char * comp_id = NULL)
    : smsc::util::CustomException("%s%sASN1 Decoding of %s failed at %d byte",
                                  comp_id ? comp_id : "", comp_id ? ": " : "",
                                  def_name, pos)
  {
    errCode = er_code;
    setExcId("ASN1DecodeError");
  }
};

}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_COMP_DEFS_HPP__ */
