#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_DEFS_HPP__
#define __SMSC_INMAN_COMP_DEFS_HPP__

#include <string>
#include <stdexcept>
#include <vector>

using std::vector;
using std::runtime_error;

namespace smsc {
namespace inman {
namespace comp{


typedef std::runtime_error EncodeError;
typedef std::runtime_error DecodeError;


//This is the base class for types of arguments, results and errors of
//OPERATIONs, which are transferred through TCAP.
class Component
{
  public:
    virtual void encode(vector<unsigned char>& buf)
    { throw EncodeError("ASN.1 encoding is not implemented"); }
    virtual void decode(const vector<unsigned char>& buf)
    { throw DecodeError("ASN.1 decoding is not implemented"); }
};


}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_COMP_DEFS_HPP__ */
