/* ************************************************************************* *
 * ROS Operations Component interface definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_COMP_DEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_COMP_DEFS_HPP__

#include <vector>

#include "util/Exception.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::util::CustomException;

class ASN1EncodeError : public CustomException {
public:
    ASN1EncodeError(const char * def_name, const char * failed_type,
                    const char * comp_id = NULL)
        : CustomException("%s%sASN1 Encoding of %s failed at %s",
                          comp_id ? comp_id : "", comp_id ? ": " : "",
                          def_name, failed_type)
    { setExcId("ASN1EncodeError"); }
};

class ASN1DecodeError : public CustomException {
public:
    ASN1DecodeError(const char * def_name, int er_code, size_t pos,
                    const char * comp_id = NULL)
        : CustomException("%s%sASN1 Decoding of %s failed at %d byte",
                          comp_id ? comp_id : "", comp_id ? ": " : "",
                          def_name, pos)
    {
        errCode = er_code;
        setExcId("ASN1DecodeError");
    }
};


//This is the base class for types of arguments, results and errors of
//OPERATIONs, which are transferred through TCAP.
class Component {
public:
    virtual void encode(std::vector<unsigned char>& buf) const throw(CustomException)
    { throw CustomException(-1, "ASN.1 encoding is not implemented"); }

    virtual void decode(const std::vector<unsigned char>& buf) throw(CustomException)
    { throw CustomException(-1, "ASN.1 decoding is not implemented"); }
};

//Implement this abstract class if Component is a primitive of MAP service and
//may be transferred over TCAP in segmented manner (see 3GPP TS 29.002 Annex C)
class MAPComponent: public Component {
public:
    virtual void mergeSegment(Component * segm) throw(CustomException)
    { throw CustomException(-2, "TC-Result-NL segmentation is not supported"); }
};


}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_COMP_DEFS_HPP__ */
