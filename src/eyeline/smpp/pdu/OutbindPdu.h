#ifndef _EYELINE_SMPP_OUTBINDPDU_H
#define _EYELINE_SMPP_OUTBINDPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace smpp {

class OutbindPdu : public RequestPdu
{
public:
    OutbindPdu() : RequestPdu(OUTBIND) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(system_id.c_str());
        tb.setCString(password.c_str());
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        system_id = fb.getCString();
        password = fb.getCString();
    }

protected:
    size_t mandatorySize() const {
        return system_id.length() + 1 + password.length() + 1;
    }

protected:
    smsc::core::buffers::FixedLengthString<16> system_id;
    smsc::core::buffers::FixedLengthString<9> password;

public: // mandatory fields access
    inline const char* getSystemId() const { return system_id.c_str(); }
    inline const char* getPassword() const { return password.c_str(); }

    inline void setSystemId(const char* value) { system_id = value; }
    inline void setPassword(const char* value) { password = value; }
};

}
}

#endif
