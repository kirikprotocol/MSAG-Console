#ifndef _EYELINE_SMPP_TESTPDU_H
#define _EYELINE_SMPP_TESTPDU_H

#include "Pdu.h"
#include "TlvDefines.h"

namespace eyeline {
namespace smpp {

const char* tlvDataTypeToString( TlvDataType pt )
{
#define PTTOSTRING(x) case(x) : return #x
    switch (pt) {
        PTTOSTRING(TLVUNDEFINED);
        PTTOSTRING(TLVUINT8);
        PTTOSTRING(TLVUINT16);
        PTTOSTRING(TLVUINT32);
        PTTOSTRING(TLVUINT64);
        PTTOSTRING(TLVCSTRING);
        PTTOSTRING(TLVOCTET);
#undef PTTOSTRING
    default: return "???";
    }
}

class PduContainer
{
public:
    PduContainer(const uint32_t int_value, const char* str_value, smsc::logger::Logger* logptr)
        : value(int_value), buf(str_value), log(logptr) {}
    ~PduContainer() {}
    virtual Pdu* getPdu() { return pdu; }
    int fillPdu(bool silent=0) {
        int result = fillBase();
        result += fillMandatory();
        result += fillOptional(silent);
        smsc_log_info(log, "Pdu created: command_id=%x %s size=%d(opt %d count %d)", pdu->getPduTypeId(),
                pduTypeToString(PduType(pdu->getPduTypeId())), pdu->evaluateSize(), pdu->optionals.evaluateSize(), pdu->optionals.count());
        return result;
    }
    int logPdu(bool silent=0) {
        int result = logBase();
        if ( !silent ) {
            result += logMandatory();
            result += logOptional();
        }
        return result;
    }
// Note: compare functions returns number of errors; normal result is 0;
    int comparePdu(Pdu *compPdu) {
        int result = compareBase(compPdu);
        result += compareMandatory(compPdu);
        result += compareOptional(compPdu);
        return result;
    }

protected:
    Pdu* pdu;
    const uint32_t value;
    const char* buf;
    smsc::logger::Logger* log;

protected:
    int fillBase() {
        pdu->setSeqNum(value);
        pdu->setStatus(value);
        return 1;
    }
    virtual int fillMandatory() { return 1; }
    int fillOptional(bool silent=0) {
        int result = 0;
        try {
            const TlvUtil::TlvDef* it = &TlvUtil::tlv_defs[0];
            for ( ; it->tag_ > 0; ++it ) {
                if ( !silent )
                    smsc_log_info(log, "%s: 0x%04x %s [%s:%d]", "Set TLV", it->tag_, it->name_, tlvDataTypeToString(it->type_), it->maxlen_);
                switch ( it->type_ ) {
                case TLVUNDEFINED :
                    pdu->optionals.setInt(it->tag_, 0, value);
                    break;
                case TLVUINT8 :
                    pdu->optionals.setInt(it->tag_, 1, value);
                    break;
                case TLVUINT16  :
                    pdu->optionals.setInt(it->tag_, 2, value);
                    break;
                case TLVUINT32  :
                    pdu->optionals.setInt(it->tag_, 4, value);
                    break;
//                case TLVUINT64  :
//                    pdu->optionals.setInt(it->tag_, 8, value);
//                  break;
                case TLVCSTRING :
                {
                    size_t maxlen = TlvUtil::tlvMaxLength(it->tag_) - 1;
                    if ( strlen(buf) > maxlen ) {
                        std::string str(buf, maxlen);
                        pdu->optionals.setString(it->tag_, str.c_str());
                    }
                    else
                        pdu->optionals.setString(it->tag_, buf);
                    break;
                }
                case TLVOCTET   :
                {
                    size_t maxlen = TlvUtil::tlvMaxLength(it->tag_);
                    size_t len = strlen(buf);
                    len = (len > maxlen) ? maxlen : len;
                    pdu->optionals.setBinary(it->tag_, len, buf);
                    break;
                }
                default:
                    break;
                }
            }
            result = 1;
        }
        catch (SmppTLVBadLenException& e) {
            smsc_log_error(log, "Set TLV error: %s", e.what());
        }
        catch (SmppException& e) {
            smsc_log_error(log, "Fill Pdu error: %s", e.what());
        }
        catch (...) {
            smsc_log_error(log, "Fill Pdu error: unknown exception");
        }
        return result;
    }
    int logBase() {
        smsc_log_info(log, "Pdu command_id=%x %s size=%d(opt %d count %d)", pdu->getPduTypeId(),
                pduTypeToString(PduType(pdu->getPduTypeId())), pdu->evaluateSize(), pdu->optionals.evaluateSize(), pdu->optionals.count());
        smsc_log_info(log, "seqNum %d status %d", pdu->getSeqNum(), pdu->getStatus());
        return 1;
    }
    virtual int logMandatory() { return 1; }
    int logOptional() {
        int result = 0;
        try {
            const TlvUtil::TlvDef* it = &TlvUtil::tlv_defs[0];
            for ( ; it->tag_ > 0; ++it ) {
                if ( !pdu->optionals.hasTag(it->tag_) ) {
                    smsc_log_error(log, "TLV not found %s", it->name_);
                    continue;
                }

                switch ( it->type_ ) {
                case TLVUNDEFINED :
                    smsc_log_info(log, "%s: [%s:%d] --", it->name_, tlvDataTypeToString(it->type_), pdu->optionals.getLen(it->tag_));
                    break;
                case TLVUINT8 :
                case TLVUINT16  :
                case TLVUINT32  :
    //            case TLVUINT64  :
                    smsc_log_info(log, "%s: [%s:%d] 0x%x", it->name_, tlvDataTypeToString(it->type_), pdu->optionals.getLen(it->tag_), pdu->optionals.getInt(it->tag_));
                    break;
                case TLVCSTRING :
                    smsc_log_info(log, "%s: [%s:%d] %s", it->name_, tlvDataTypeToString(it->type_), pdu->optionals.getLen(it->tag_), pdu->optionals.getString(it->tag_));
                    break;
                case TLVOCTET   :
                {
                    size_t len = 0;
                    const void* tmp = pdu->optionals.getBinary(it->tag_, len);
                    std::string str(reinterpret_cast<const char*>(tmp), len);
                    smsc_log_info(log, "%s: [%s:%d] %s", it->name_, tlvDataTypeToString(it->type_), len, str.c_str());
                    break;
                }
                default:
                    break;
                }
            }
            result = 1;
        }
        catch (SmppException& e) {
            smsc_log_error(log, "%s: %s", "Fill Pdu error", e.what());
        }
        catch (...) {
            smsc_log_error(log, "Fill Pdu error: unknown exception");
        }
        return result;
    }

#define CHECK_TLV_STRING(x) if (strcmp(pdu->x(), compPdu->x()) != 0) ++result
#define CHECK_TLV_INT(x) if (pdu->x() != compPdu->x()) ++result
    int compareBase(Pdu *compPdu) {
        int result = 0;
        CHECK_TLV_INT(getPduTypeId);
        CHECK_TLV_INT(getSeqNum);
        CHECK_TLV_INT(getStatus);
        return result;
    }
#undef CHECK_TLV_INT
#undef CHECK_TLV_STRING

    virtual int compareMandatory(Pdu *compPdu) { return 0; }

    int compareOptional(Pdu *compPdu) {
        int result = 0;
        const TlvUtil::TlvDef* it = &TlvUtil::tlv_defs[0];
        for ( ; it->tag_ > 0; ++it ) {
            if ( !pdu->optionals.hasTag(it->tag_) ) {
                ++result;
                smsc_log_error(log, "Fault %s: not found in Pdu1", it->name_);
                continue;
            }
            if ( !compPdu->optionals.hasTag(it->tag_) ) {
                ++result;
                smsc_log_error(log, "Fault %s: not found in Pdu2", it->name_);
                continue;
            }
            switch ( it->type_ ) {
            case TLVUNDEFINED :
                break;
            case TLVUINT8 :
            case TLVUINT16  :
            case TLVUINT32  :
    //        case TLVUINT64  :
                if ( pdu->optionals.getInt(it->tag_) != compPdu->optionals.getInt(it->tag_) ) {
                    ++result;
                    smsc_log_error(log, "Fault %s: [%s:%d] 0x%x [%s:%d] 0x%x", it->name_,
                            tlvDataTypeToString(it->type_), pdu->optionals.getLen(it->tag_), pdu->optionals.getInt(it->tag_),
                            tlvDataTypeToString(it->type_), compPdu->optionals.getLen(it->tag_), compPdu->optionals.getInt(it->tag_)
                            );
                }
                break;
            case TLVCSTRING :
                if ( strcmp(pdu->optionals.getString(it->tag_), compPdu->optionals.getString(it->tag_)) != 0 ) {
                    ++result;
                    smsc_log_error(log, "Fault %s: [%s:%d] %s [%s:%d] %s", it->name_,
                        tlvDataTypeToString(it->type_), pdu->optionals.getLen(it->tag_), pdu->optionals.getString(it->tag_),
                        tlvDataTypeToString(it->type_), compPdu->optionals.getLen(it->tag_), compPdu->optionals.getString(it->tag_)
                        );
                }
                break;
            case TLVOCTET   :
            {
                size_t len1 = 0, len2 = 0;
                const void* tmp = pdu->optionals.getBinary(it->tag_, len1);
                std::string str1(reinterpret_cast<const char*>(tmp), len1);
                tmp = compPdu->optionals.getBinary(it->tag_, len2);
                std::string str2(reinterpret_cast<const char*>(tmp), len2);
                if ( str1 != str2 ) {
                    ++result;
                    smsc_log_error(log, "Fault %s: [%s:%d] %s [%s:%d] %s", it->name_,
                        tlvDataTypeToString(it->type_), len1, str1.c_str(),
                        tlvDataTypeToString(it->type_), len2, str2.c_str()
                        );
                }
                break;
            }
            default:
                ++result;
                smsc_log_error(log, "Fault %s: invalid data type %d", it->name_, it->type_);
                break;
            }
        }
        return result;
    }
};


}
}

#endif
