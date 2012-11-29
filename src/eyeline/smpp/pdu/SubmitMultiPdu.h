#ifndef _EYELINE_SMPP_SUBMITMULTIPDU_H
#define _EYELINE_SMPP_SUBMITMULTIPDU_H

#include "RequestPdu.h"
#include "ResponsePdu.h"
#include "PduAddr.h"
#include "eyeline/smpp/SmppException.h"
#include <vector>

namespace eyeline {
namespace smpp {

class SubmitMultiRespPdu : public ResponsePdu
{
protected:
    struct UnsuccessSme {
        addr_params_t<ADDRLEN21> dest_addr;
        uint32_t error_status_code;

        UnsuccessSme() { dest_addr.ton=0; dest_addr.npi=0; error_status_code=0; }
        UnsuccessSme(uint8_t dat, uint8_t dan, const char* da, uint32_t esc)
        {
            dest_addr.ton = dat;
            dest_addr.npi = dan;
            dest_addr.addr = da;
            error_status_code = esc;
        }

        // just evaluate minimum size of struct
        // static const size_t size() {
        // return sizeof(uint32_t) + sizeof(uint32_t) + 1 + sizeof(uint32_t);
        // }
        size_t evaluateSize() const {
            return dest_addr.size() + sizeof(error_status_code);
        }

        void encode( eyeline::informer::ToBuf& tb ) const 
        {
            dest_addr.encode(tb);
            tb.ncset32(error_status_code);
        }

        void decode( eyeline::informer::FromBuf& fb ) 
        {
            dest_addr.decode(fb);
            error_status_code = fb.get32();
        }
    };

protected:  // mandatory fields
    typedef std::vector<UnsuccessSme> unsList;

    smsc::core::buffers::FixedLengthString<65> message_id;
    unsList unsuccess_sme;

public:
    SubmitMultiRespPdu( uint32_t status, uint32_t seqNum ) : ResponsePdu(status,seqNum) {
        pduTypeId_ = SUBMIT_MULTI_RESP;
    }

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(message_id.c_str());
        const uint8_t nu = uint8_t(unsuccess_sme.size());
        tb.ncset8(nu);
        // if ( no_unsuccess != unsuccess_sme.size() ) {
        // throw SmppException("invalid data no_unsuccess=%d (!=%d) (SMPP5.0: 4.2.3.2)", no_unsuccess, unsuccess_sme.size());
        // }
        if ( nu ) {
            for ( unsList::const_iterator iter = unsuccess_sme.begin();
                  iter != unsuccess_sme.end(); ++iter ) {
                iter->encode(tb);
            }
        }
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) {
        message_id = fb.getCString();
        const uint8_t nu = fb.get8();
        unsuccess_sme.clear();
        while ( unsuccess_sme.size() < nu ) {
            UnsuccessSme tmp;
            tmp.decode(fb);
            unsuccess_sme.push_back(tmp);
        }
    }

protected:
    size_t mandatorySize() const {
        size_t result = message_id.length() + 1 + sizeof(uint8_t);
        for (unsList::const_iterator iter = unsuccess_sme.begin(); iter != unsuccess_sme.end(); ++iter ) {
            result += iter->evaluateSize();
        }
        return result;
    }

public: // mandatory fields access
    inline const char* getMessageId() const { return message_id.c_str(); }
    inline const uint8_t getNoUnsuccess() const { return uint8_t(unsuccess_sme.size()); }
    bool getUnsuccessSme( unsigned int index,
                          uint8_t& DestAddrTon,
                          uint8_t& DestAddrNpi,
                          std::string& DestAddr,
                          uint32_t& ErrorStatusCode ) const {
        if ( index >= unsuccess_sme.size() )
            return false;
        const UnsuccessSme& tmp = unsuccess_sme[index];
        DestAddrTon = tmp.dest_addr.ton;
        DestAddrNpi = tmp.dest_addr.npi;
        DestAddr = tmp.dest_addr.addr.c_str();
        ErrorStatusCode = tmp.error_status_code;
        return true;
    }

    inline void setMessageId(const char* value) { message_id = value; }
    inline void addUnsuccessSme( uint8_t DestAddrTon,
                                 uint8_t DestAddrNpi,
                                 const char* DestAddr,
                                 uint32_t ErrorStatusCode ) {
        if ( unsuccess_sme.size() >= 255 ) {
            throw SmppException("too many unsuccess sme");
        }
        unsuccess_sme.push_back(UnsuccessSme(DestAddrTon, DestAddrNpi, DestAddr, ErrorStatusCode));
    }

public: // optional fields access, SMPP5.0 ¤4.2.5
    const char* getAdditionalStatusInfoText() { return optionals.getString(TAG_ADDITIONAL_STATUS_INFO_TEXT); }
    const uint8_t getDeliveryFailureReason() { return optionals.getInt(TAG_DELIVERY_FAILURE_REASON); }
    const uint8_t getDpfResult() { return optionals.getInt(TAG_DPF_RESULT); }
    const void* getNetworkErrorCode(size_t& len) { return optionals.getBinary(TAG_NETWORK_ERROR_CODE, len); }

    void setAdditionalStatusInfoText(const char* value) { optionals.setString(TAG_ADDITIONAL_STATUS_INFO_TEXT, value); }
    void setDeliveryFailureReason(const uint8_t value) { optionals.setInt(TAG_DELIVERY_FAILURE_REASON, 1, uint32_t(value)); }
    void setDpfResult(const uint8_t value) { optionals.setInt(TAG_DPF_RESULT, 1, uint32_t(value)); }
    void setNetworkErrorCode(size_t len, const void* value) { optionals.setBinary(TAG_NETWORK_ERROR_CODE, len, value); }
};


class SubmitMultiPdu : public RequestPdu
{
protected: // mandatory fields
    smsc::core::buffers::FixedLengthString<6> service_type;
    addr_params_t<ADDRLEN21> source_addr;
    uint8_t number_of_dests;
    uint8_t dest_flag;  //0x01 (SME Address) or 0x02 (Distribution List)
    addr_params_t<ADDRLEN21> dest_addr;
    uint8_t esm_class;
    uint8_t protocol_id;
    uint8_t priority_flag;
    smsc::core::buffers::FixedLengthString<17> schedule_delivery_time;
    smsc::core::buffers::FixedLengthString<17> validity_period;
    uint8_t registered_delivery;
    uint8_t replace_if_present_flag;
    uint8_t data_coding;
    uint8_t sm_default_msg_id;
    uint8_t sm_length;
    smsc::core::buffers::FixedLengthString<256> short_message;  // 255 + trailing '\0'

public:
    SubmitMultiPdu() : RequestPdu(SUBMIT_MULTI) {}

    void encodeMandatoryFields( eyeline::informer::ToBuf& tb ) const 
    {
        tb.setCString(service_type.c_str());
        source_addr.encode(tb);
        tb.ncset8(number_of_dests);
        tb.ncset8(dest_flag);
        if ( 0x01 == dest_flag ) {
            dest_addr.encode(tb);
        } else {
            if ( 0x02 == dest_flag ) {
                tb.setCString(dest_addr.addr.c_str());
            } else {
                throw SmppException("invalid value dest_flag=%x (SMPP5.0: 4.7.9)", dest_flag);
            }
        }
        tb.ncset8(esm_class);
        tb.ncset8(protocol_id);
        tb.ncset8(priority_flag);
        tb.setCString(schedule_delivery_time.c_str());
        tb.setCString(validity_period.c_str());
        tb.ncset8(registered_delivery);
        tb.ncset8(replace_if_present_flag);
        tb.ncset8(data_coding);
        tb.ncset8(sm_default_msg_id);
        tb.ncset8(sm_length);
        tb.copy(size_t(sm_length),short_message.str);
    }

    void decodeMandatoryFields( eyeline::informer::FromBuf& fb ) 
    {
        service_type = fb.getCString();
        source_addr.decode(fb);
        number_of_dests = fb.get8();
        dest_flag = fb.get8();

        if ( 0x01 == dest_flag ) {
            dest_addr.decode(fb);
        } else {
            if ( 0x02 == dest_flag ) {
                dest_addr.addr = fb.getCString();
            } else {
                throw SmppException("invalid value dest_flag=%x (SMPP5.0: 4.7.9)", dest_flag);
            }
        }
        esm_class = fb.get8();
        protocol_id = fb.get8();
        priority_flag = fb.get8();
        schedule_delivery_time = fb.getCString();
        validity_period = fb.getCString();
        registered_delivery = fb.get8();
        replace_if_present_flag = fb.get8();
        data_coding = fb.get8();
        sm_default_msg_id = fb.get8();
        const size_t sz = sm_length = fb.get8();
        short_message.assign(fb.skip(sz),sz);
    }

protected:
    size_t mandatorySize() const {
        return ( service_type.length() + 1 +
                 source_addr.size() +
                 ( ( 0x01 == dest_flag ) ? dest_addr.size() : ( dest_addr.addr.length() + 1) ) +
                 sizeof(esm_class) + sizeof(protocol_id) +
                 sizeof(priority_flag) +
                 schedule_delivery_time.length() + 1 +
                 validity_period.length() + 1 +
                 sizeof(registered_delivery) +
                 sizeof(replace_if_present_flag) +
                 sizeof(data_coding) + sizeof(sm_default_msg_id) +
                 sizeof(sm_length) + sm_length );
    }

public: // mandatory fields access
    inline const char* getServiceType() const { return service_type.c_str(); }
    inline const uint8_t getSourceAddrTon() const { return source_addr.ton; }
    inline const uint8_t getSourceAddrNpi() const { return source_addr.npi; }
    inline const char* getSourceAddr() const { return source_addr.addr.c_str(); }
    inline const uint8_t getNumberOfDests() const { return number_of_dests; }
    inline const uint8_t getDestFlag() const { return dest_flag; }
    inline const uint8_t getDestAddrTon() const { return dest_addr.ton; }
    inline const uint8_t getDestAddrNpi() const { return dest_addr.npi; }
    inline const char* getDestAddr() const { return dest_addr.addr.c_str(); }
    inline const uint8_t getEsmClass() const { return esm_class; }
    inline const uint8_t getProtocolId() const { return protocol_id; }
    inline const uint8_t getPriorityFlag() const { return priority_flag; }
    inline const char* getScheduleDeliveryTime() const { return schedule_delivery_time.c_str(); }
    inline const char* getValidityPeriod() const { return validity_period.c_str(); }
    inline const uint8_t getRegisteredDelivery() const { return registered_delivery; }
    inline const uint8_t getReplaceIfPresentFlag() const { return replace_if_present_flag; }
    inline const uint8_t getDataCoding() const { return data_coding; }
    inline const uint8_t getSmDefaultMsgId() const { return sm_default_msg_id; }
    inline const uint8_t getSmLength() const { return sm_length; }
    inline const char* getShortMessage() const { return short_message.str; }

    inline void setServiceType(const char* value) { service_type = value; }
    inline void setSourceAddrTon(const uint8_t value) { source_addr.ton = value; }
    inline void setSourceAddrNpi(const uint8_t value) { source_addr.npi = value; }
    inline void setSourceAddr(const char* value) { source_addr.addr = value; }
    inline void setNumberOfDests(const uint8_t value) { number_of_dests = value; }
    inline void setDestFlag(const uint8_t value) { dest_flag = value; }
    inline void setDestAddrTon(const uint8_t value) { dest_addr.ton = value; }
    inline void setDestAddrNpi(const uint8_t value) { dest_addr.npi = value; }
    inline void setDestAddr(const char* value) { dest_addr.addr = value; }
    inline void setEsmClass(const uint8_t value) { esm_class = value; }
    inline void setProtocolId(const uint8_t value) { protocol_id = value; }
    inline void setPriorityFlag(const uint8_t value) { priority_flag = value; }
    inline void setScheduleDeliveryTime(const char* value) { schedule_delivery_time = value; }
    inline void setValidityPeriod(const char* value) { validity_period = value; }
    inline void setRegisteredDelivery(const uint8_t value) { registered_delivery = value; }
    inline void setReplaceIfPresentFlag(const uint8_t value) { replace_if_present_flag = value; }
    inline void setDataCoding(const uint8_t value) { data_coding = value; }
    inline void setSmDefaultMsgId(const uint8_t value) { sm_default_msg_id = value; }
    inline void setShortMessage(const char* value,
                                uint8_t len ) {
        const size_t sz = sm_length = len;
        short_message.assign(value,sz);
    }

public: // optional fields access, SMPP5.0 ¤4.2.4
    const bool getAlertOnMsgDelivery() { return optionals.hasTag(TAG_ALERT_ON_MESSAGE_DELIVERY); }

    const uint8_t getCallbackNumPresInd() { return optionals.getInt(TAG_CALLBACK_NUM_PRES_IND); }
    const uint8_t getDestAddrNpResolution() { return optionals.getInt(TAG_DEST_ADDR_NP_RESOLUTION); }
    const uint8_t getDestAddrSubunit() { return optionals.getInt(TAG_DEST_ADDR_SUBUNIT); }
    const uint8_t getDestBearerType() { return optionals.getInt(TAG_DEST_BEARER_TYPE); }
    const uint8_t getDestNetworkType() { return optionals.getInt(TAG_DEST_NETWORK_TYPE); }
    const uint8_t getDisplayTime() { return optionals.getInt(TAG_DISPLAY_TIME); }
    const uint8_t getItsReplyType() { return optionals.getInt(TAG_ITS_REPLY_TYPE); }
    const uint8_t getLanguageIndicator() { return optionals.getInt(TAG_LANGUAGE_INDICATOR); }
    const uint8_t getMoreMessagesToSend() { return optionals.getInt(TAG_MORE_MESSAGES_TO_SEND); }
    const uint8_t getMsMsgWaitFacilities() { return optionals.getInt(TAG_MS_MSG_WAIT_FACILITIES); }
    const uint8_t getNumberOfMessages() { return optionals.getInt(TAG_NUMBER_OF_MESSAGES); }
    const uint8_t getPayloadType() { return optionals.getInt(TAG_PAYLOAD_TYPE); }
    const uint8_t getPrivacyIndicator() { return optionals.getInt(TAG_PRIVACY_INDICATOR); }
    const uint8_t getSarSegmentSeqnum() { return optionals.getInt(TAG_SAR_SEGMENT_SEQNUM); }
    const uint8_t getSarTotalSegments() { return optionals.getInt(TAG_SAR_TOTAL_SEGMENTS); }
    const uint8_t getSetDpf() { return optionals.getInt(TAG_SET_DPF); }
    const uint8_t getSourceAddrSubunit() { return optionals.getInt(TAG_SOURCE_ADDR_SUBUNIT); }
    const uint8_t getSourceBearerType() { return optionals.getInt(TAG_SOURCE_BEARER_TYPE); }
    const uint8_t getSourceNetworkType() { return optionals.getInt(TAG_SOURCE_NETWORK_TYPE); }
    const uint8_t getSourceTelematicsId() { return optionals.getInt(TAG_SOURCE_TELEMATICS_ID); }
    const uint8_t getUserResponseCode() { return optionals.getInt(TAG_USER_RESPONSE_CODE); }
    const uint8_t getUssdServiceOp() { return optionals.getInt(TAG_USSD_SERVICE_OP); }
    const uint16_t getSarMsgRefNum() { return optionals.getInt(TAG_SAR_MSG_REF_NUM); }
    const uint16_t getSmsSignal() { return optionals.getInt(TAG_SMS_SIGNAL); }
    const uint16_t getDestTelematicsId() { return optionals.getInt(TAG_DEST_TELEMATICS_ID); }
    const uint16_t getDestPort() { return optionals.getInt(TAG_DEST_PORT); }
    const uint16_t getSourcePort() { return optionals.getInt(TAG_SOURCE_PORT); }
    const uint16_t getUserMessageReference() { return optionals.getInt(TAG_USER_MESSAGE_REFERENCE); }
    const uint32_t getQosTimeToLive() { return optionals.getInt(TAG_QOS_TIME_TO_LIVE); }

    const char* getDestNetworkId() { return optionals.getString(TAG_DEST_NETWORK_ID); }
    const char* getSourceNetworkId() { return optionals.getString(TAG_SOURCE_NETWORK_ID); }

    const void* getBillingIdentification(size_t& len) { return optionals.getBinary(TAG_BILLING_IDENTIFICATION, len); }
    const void* getCallbackNum(size_t& len) { return optionals.getBinary(TAG_CALLBACK_NUM, len); }
    const void* getCallbackNumAtag(size_t& len) { return optionals.getBinary(TAG_CALLBACK_NUM_ATAG, len); }
    const void* getDestAddrNpCountry(size_t& len) { return optionals.getBinary(TAG_DEST_ADDR_NP_COUNTRY, len); }
    const void* getDestAddrNpInformation(size_t& len) { return optionals.getBinary(TAG_DEST_ADDR_NP_INFORMATION, len); }
    const void* getDestNodeId(size_t& len) { return optionals.getBinary(TAG_DEST_NODE_ID, len); }
    const void* getDestSubaddress(size_t& len) { return optionals.getBinary(TAG_DEST_SUBADDRESS, len); }
    const void* getItsSessionInfo(size_t& len) { return optionals.getBinary(TAG_ITS_SESSION_INFO, len); }
    const void* getMessagePayload(size_t& len) { return optionals.getBinary(TAG_MESSAGE_PAYLOAD, len); }
    const void* getMsValidity(size_t& len) { return optionals.getBinary(TAG_MS_VALIDITY, len); }
    const void* getSourceNodeId(size_t& len) { return optionals.getBinary(TAG_SOURCE_NODE_ID, len); }
    const void* getSourceSubaddress(size_t& len) { return optionals.getBinary(TAG_SOURCE_SUBADDRESS, len); }

    void setAlertOnMsgDelivery() { optionals.setInt(TAG_ALERT_ON_MESSAGE_DELIVERY, 0, 0); }

    void setCallbackNumPresInd(const uint8_t value) { optionals.setInt(TAG_CALLBACK_NUM_PRES_IND, 1, uint32_t(value)); }
    void setDestAddrNpResolution(const uint8_t value) { optionals.setInt(TAG_DEST_ADDR_NP_RESOLUTION, 1, uint32_t(value)); }
    void setDestAddrSubunit(const uint8_t value) { optionals.setInt(TAG_DEST_ADDR_SUBUNIT, 1, uint32_t(value)); }
    void setDestBearerType(const uint8_t value) { optionals.setInt(TAG_DEST_BEARER_TYPE, 1, uint32_t(value)); }
    void setDestNetworkType(const uint8_t value) { optionals.setInt(TAG_DEST_NETWORK_TYPE, 1, uint32_t(value)); }
    void setDisplayTime(const uint8_t value) { optionals.setInt(TAG_DISPLAY_TIME, 1, uint32_t(value)); }
    void setItsReplyType(const uint8_t value) { optionals.setInt(TAG_ITS_REPLY_TYPE, 1, uint32_t(value)); }
    void setLanguageIndicator(const uint8_t value) { optionals.setInt(TAG_LANGUAGE_INDICATOR, 1, uint32_t(value)); }
    void setMoreMessagesToSend(const uint8_t value) { optionals.setInt(TAG_MORE_MESSAGES_TO_SEND, 1, uint32_t(value)); }
    void setMsMsgWaitFacilities(const uint8_t value) { optionals.setInt(TAG_MS_MSG_WAIT_FACILITIES, 1, uint32_t(value)); }
    void setNumberOfMessages(const uint8_t value) { optionals.setInt(TAG_NUMBER_OF_MESSAGES, 1, uint32_t(value)); }
    void setPayloadType(const uint8_t value) { optionals.setInt(TAG_PAYLOAD_TYPE, 1, uint32_t(value)); }
    void setPrivacyIndicator(const uint8_t value) { optionals.setInt(TAG_PRIVACY_INDICATOR, 1, uint32_t(value)); }
    void setSarSegmentSeqnum(const uint8_t value) { optionals.setInt(TAG_SAR_SEGMENT_SEQNUM, 1, uint32_t(value)); }
    void setSarTotalSegments(const uint8_t value) { optionals.setInt(TAG_SAR_TOTAL_SEGMENTS, 1, uint32_t(value)); }
    void setSetDpf(const uint8_t value) { optionals.setInt(TAG_SET_DPF, 1, uint32_t(value)); }
    void setSourceAddrSubunit(const uint8_t value) { optionals.setInt(TAG_SOURCE_ADDR_SUBUNIT, 1, uint32_t(value)); }
    void setSourceBearerType(const uint8_t value) { optionals.setInt(TAG_SOURCE_BEARER_TYPE, 1, uint32_t(value)); }
    void setSourceNetworkType(const uint8_t value) { optionals.setInt(TAG_SOURCE_NETWORK_TYPE, 1, uint32_t(value)); }
    void setSourceTelematicsId(const uint8_t value) { optionals.setInt(TAG_SOURCE_TELEMATICS_ID, 1, uint32_t(value)); }
    void setUserResponseCode(const uint8_t value) { optionals.setInt(TAG_USER_RESPONSE_CODE, 1, uint32_t(value)); }
    void setUssdServiceOp(const uint8_t value) { optionals.setInt(TAG_USSD_SERVICE_OP, 1, uint32_t(value)); }
    void setSarMsgRefNum(const uint16_t value) { optionals.setInt(TAG_SAR_MSG_REF_NUM, 1, uint32_t(value)); }
    void setSmsSignal(const uint16_t value) { optionals.setInt(TAG_SMS_SIGNAL, 1, uint32_t(value)); }
    void setDestTelematicsId(const uint16_t value) { optionals.setInt(TAG_DEST_TELEMATICS_ID, 1, uint32_t(value)); }
    void setDestPort(const uint16_t value) { optionals.setInt(TAG_DEST_PORT, 1, uint32_t(value)); }
    void setSourcePort(const uint16_t value) { optionals.setInt(TAG_SOURCE_PORT, 1, uint32_t(value)); }
    void setUserMessageReference(const uint16_t value) { optionals.setInt(TAG_USER_MESSAGE_REFERENCE, 1, uint32_t(value)); }
    void setQosTimeToLive(const uint32_t value) { optionals.setInt(TAG_QOS_TIME_TO_LIVE, 1, value); }

    void setDestNetworkId(const char* value) { optionals.setString(TAG_DEST_NETWORK_ID, value); }
    void setSourceNetworkId(const char* value) { optionals.setString(TAG_SOURCE_NETWORK_ID, value); }

    void setBillingIdentification(size_t len, const void* value) { optionals.setBinary(TAG_BILLING_IDENTIFICATION, len, value); }
    void setCallbackNum(size_t len, const void* value) { optionals.setBinary(TAG_CALLBACK_NUM, len, value); }
    void setCallbackNumAtag(size_t len, const void* value) { optionals.setBinary(TAG_CALLBACK_NUM_ATAG, len, value); }
    void setDestAddrNpCountry(size_t len, const void* value) { optionals.setBinary(TAG_DEST_ADDR_NP_COUNTRY, len, value); }
    void setDestAddrNpInformation(size_t len, const void* value) { optionals.setBinary(TAG_DEST_ADDR_NP_INFORMATION, len, value); }
    void setDestNodeId(size_t len, const void* value) { optionals.setBinary(TAG_DEST_NODE_ID, len, value); }
    void setDestSubaddress(size_t len, const void* value) { optionals.setBinary(TAG_DEST_SUBADDRESS, len, value); }
    void setItsSessionInfo(size_t len, const void* value) { optionals.setBinary(TAG_ITS_SESSION_INFO, len, value); }
    void setMessagePayload(size_t len, const void* value) { optionals.setBinary(TAG_MESSAGE_PAYLOAD, len, value); }
    void setMsValidity(size_t len, const void* value) { optionals.setBinary(TAG_MS_VALIDITY, len, value); }
    void setSourceNodeId(size_t len, const void* value) { optionals.setBinary(TAG_SOURCE_NODE_ID, len, value); }
    void setSourceSubaddress(size_t len, const void* value) { optionals.setBinary(TAG_SOURCE_SUBADDRESS, len, value); }
};


}
}

#endif
