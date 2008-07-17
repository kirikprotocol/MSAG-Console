#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_COMP_HPP__
#define __SMSC_MTSMSME_COMP_COMP_HPP__

#include <vector>

namespace smsc{ namespace mtsmsme{ namespace comp{

//This is the base class for types of arguments, results and errors of
//OPERATIONs, which are transferred through TCAP.

using std::vector;
class CompIF {
public:
    virtual void encode(vector<unsigned char>& buf) = 0;
    virtual void decode(const vector<unsigned char>& buf) = 0;
};
class EmptyComp: public CompIF {
  public:
    void encode(vector<unsigned char>& buf) {}
    void decode(const vector<unsigned char>& buf) {}
};
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }

#endif /* __SMSC_MTSMSME_COMP_COMP_HPP__  */
