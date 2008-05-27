#ident "$Id$"
#ifndef __SMSC_MTSMSME_COMP_PROVIDEROAMINGNUMBER_HPP__
#define __SMSC_MTSMSME_COMP_PROVIDEROAMINGNUMBER_HPP__

#include "mtsmsme/comp/Component.hpp"
#include "mtsmsme/processor/util.hpp"
#include "ProvideRoamingNumberRes.h"
#include <string>

namespace smsc{ namespace mtsmsme{ namespace comp{

using smsc::mtsmsme::comp::CompIF;
using std::string;

class ProvideRoamingNumberResp: public  CompIF {
  public:
    void setRoamingNumber(const string& number);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
private:
    ProvideRoamingNumberRes_t arg;
    OCTET_STRING_DECL(_number,20);
};
class ProvideRoamingNumberRespV1: public  CompIF {
  public:
    void setRoamingNumber(const string& number);
    virtual void encode(vector<unsigned char>& buf);
    virtual void decode(const vector<unsigned char>& buf);
private:
    OCTET_STRING_DECL(_number,20);
};
/* namespace comp */ } /* namespace mtsmsme */ } /* namespace smsc */ }

#endif /* __SMSC_MTSMSME_COMP_PROVIDEROAMINGNUMBER_HPP__ */
