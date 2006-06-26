#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_DEFS_HPP__
#define __SMSC_INMAN_COMP_DEFS_HPP__

#include <vector>

#include "inman/common/errors.hpp"
#include "inman/common/adrutil.hpp"

using std::vector;
using smsc::inman::common::CustomException;

namespace smsc {
namespace inman {
namespace comp {

typedef struct {
    unsigned      serviceKey;
    TonNpiAddress scfAddress;
} MAPSCFinfo;

class ASN1EncodeError : public CustomException {
public:
    ASN1EncodeError(const char * def_name, const char * failed_type)
        : CustomException(format("ASN1 Encoding of %s failed at %s",
                                 def_name, failed_type).c_str(), -1, NULL)
    { 
        setExcId("ASN1EncodeError");
    }
};

class ASN1DecodeError : public CustomException {
public:
    ASN1DecodeError(const char * def_name, int er_code, size_t pos)
        : CustomException(format("ASN1 Decoding of %s failed at %d byte",
                                 def_name, pos).c_str(), er_code, NULL)
    { 
        setExcId("ASN1DecodeError");
    }
};


//This is the base class for types of arguments, results and errors of
//OPERATIONs, which are transferred through TCAP.
class Component {
public:
    virtual void encode(vector<unsigned char>& buf) throw(CustomException)
    { throw CustomException("ASN.1 encoding is not implemented", -1, NULL); }

    virtual void decode(const vector<unsigned char>& buf) throw(CustomException)
    { throw CustomException("ASN.1 decoding is not implemented", -1, NULL); }
};

//Implement this abstract class if Component is a primitive of MAP service and
//may be transferred over TCAP in segmented manner (see 3GPP TS 29.002 Annex C)
class MAPComponent: public Component {
public:
    virtual void mergeSegment(const Component * segm) throw(CustomException)
    { throw CustomException("TC-Result-NL segmentation is not supported", -1, NULL); }
};


}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_COMP_DEFS_HPP__ */
