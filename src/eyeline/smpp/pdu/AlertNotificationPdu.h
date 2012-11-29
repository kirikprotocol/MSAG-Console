#ifndef _EYELINE_SMPP_ALERTNOTIFICATIONPDU_H
#define _EYELINE_SMPP_ALERTNOTIFICATIONPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/FixedLengthString.hpp"


namespace eyeline {
namespace smpp {

class AlertNotificationPdu : public RequestPdu
{
protected: //mandatory fields
    addr_params_t<ADDRLEN65> source_addr;
    addr_params_t<ADDRLEN65> esme_addr;

public:
    AlertNotificationPdu() : RequestPdu(ALERT_NOTIFICATION) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const {
        source_addr.encode(tb);
        esme_addr.encode(tb);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) {
        source_addr.decode(fb);
        esme_addr.decode(fb);
    }

protected:
    size_t mandatorySize() const {
        return source_addr.size() + esme_addr.size();
    }

public:    //mandatory fields access
    inline const uint8_t getSourceAddrTon() const { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const { return source_addr.npi; }
    inline const char* getSourceAddr() const { return source_addr.addr.c_str(); }
    inline const uint8_t getEsmeAddrTon() const { return esme_addr.ton; }
    inline const uint8_t getEsmeAddrNpi() const { return esme_addr.npi; }
    inline const char* getEsmeAddr() const { return esme_addr.addr.c_str(); }

    inline void setSourceAddrTon(const uint8_t value) { source_addr.ton = value; }
    inline void setSourceAddrNpi(const uint8_t value) { source_addr.npi = value; }
    inline void setSourceAddr(const char* value) { source_addr.addr = value; }
    inline void setEsmeAddrTon(const uint8_t value) { esme_addr.ton = value; }
    inline void setEsmeAddrNpi(const uint8_t value) { esme_addr.npi = value; }
    inline void setEsmeAddr(const char* value) { esme_addr.addr = value; }

public: // optional fields access, SMPP5.0 ¤4.8.4.39
    uint8_t getMsAvailabilityStatus() { return optionals.getInt(TAG_MS_AVAILABILITY_STATUS); }
    void setMsAvailabilityStatus(const uint8_t value) { optionals.setInt(TAG_MS_AVAILABILITY_STATUS, 1, uint32_t(value)); }
};

}
}

#endif
