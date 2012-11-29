#ifndef _EYELINE_SMPP_BINDPDU_H
#define _EYELINE_SMPP_BINDPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"


namespace eyeline {
namespace smpp {

class BindRespPdu : public ResponsePdu
{
public:
    BindRespPdu( PduType pt, uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = pt;
    }

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const {
        tb.setCString(system_id.c_str());
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) {
        system_id = fb.getCString();
    }

protected:
    size_t mandatorySize() const { return system_id.length() + 1; }

protected:
    smsc::core::buffers::FixedLengthString<16> system_id;

public: // tlv access, SMPP5.0 ¤4.8.4.51
    uint8_t getScInterfaceVersion() { return optionals.getInt(TAG_SC_INTERFACE_VERSION); }
    void setScInterfaceVersion(const uint8_t value) { optionals.setInt(TAG_SC_INTERFACE_VERSION, 1, uint32_t(value)); }
};


class BindPdu : public RequestPdu
{
protected: //mandatory fields
    smsc::core::buffers::FixedLengthString<16> system_id;
    smsc::core::buffers::FixedLengthString<9> password;
    smsc::core::buffers::FixedLengthString<13> system_type;
    uint8_t interface_version;
    addr_params_t<ADDRLEN41> address_range;

public:
    inline BindPdu( BindMode bm = BINDMODE_TRANSCEIVER ) :
    RequestPdu(GENERIC_NACK), interface_version(0) {
        setBindMode(bm);
        address_range.ton = 0;
        address_range.npi = 0;
    }

    inline void setBindMode( BindMode bm ) {
        pduTypeId_ = bindModeToPduType(bm);
    }

    inline BindMode getBindMode() const {
        return bindPduTypeToMode(PduType(pduTypeId_));
    }

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
    	tb.setCString(system_id.c_str());
    	tb.setCString(password.c_str());
    	tb.setCString(system_type.c_str());
        tb.ncset8(interface_version);
        address_range.encode(tb);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
    	system_id = fb.getCString();
    	password = fb.getCString();
    	system_type = fb.getCString();
    	interface_version = fb.get8();
    	address_range.decode(fb);
    }

    inline const char* getSystemId() const { return system_id.c_str(); }
    inline const char* getPassword() const { return password.c_str(); }
    inline const char* getSystemType() const { return system_type.c_str(); }
    inline const uint8_t getInterfaceVersion() const { return interface_version; }
    inline const uint8_t getAddrTon() const { return address_range.ton; }
    inline const uint8_t getAddrNpi() const { return address_range.npi; }
    inline const char* getAddressRange() const { return address_range.addr.c_str(); }

    inline void setSystemId(const char* value) { system_id = value; }
    inline void setPassword(const char* value) { password = value; }
    inline void setSystemType(const char* value) { system_type = value; }
    inline void setInterfaceVersion(const uint8_t value) { interface_version = value; }
    void setAddressRange(uint8_t ton, uint8_t npi, const char* value)
    {
        address_range.ton = ton;
        address_range.npi = npi;
        address_range.addr = value;
    }

    void setAddressRange(const char* value)
    {
        address_range.setAddr(value);
    }

    const char* toString( char* buf, size_t bsz, size_t* written = 0 ) const
    {
        size_t wr;
        Pdu::toString(buf,bsz,&wr);
        bsz -= wr;
        if (bsz > 0) {
            int res = snprintf(buf+wr,bsz,",sys=%s,pwd=%s,typ=%s,ver=%u,ton=%u,npi=%u,rng=%s",
                               system_id.c_str(),
                               password.c_str(),
                               system_type.c_str(),
                               interface_version,
                               address_range.ton, address_range.npi,
                               address_range.addr.c_str());
            if (res<=0) {
                buf[wr] = '\0';
                res = 0;
            } else if ( size_t(res) >= bsz ) {
                res = int(bsz-1);
            }
            wr += res;
        }
        if (written) {
            *written = wr;
        }
        return buf;
    }

protected:
    size_t mandatorySize() const 
    {
        return system_id.length() + 1 + password.length() + 1 + system_type.length() + 1
            + sizeof(interface_version) + address_range.size();
    }
};

}
}

#endif
