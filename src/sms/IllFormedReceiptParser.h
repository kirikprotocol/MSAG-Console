#ifndef _SMSC_SMS_ILLFORMEDRECEIPTPARSER_H
#define _SMSC_SMS_ILLFORMEDRECEIPTPARSER_H

#include "util/regexp/RegExp.hpp"
#include "util/smstext.h"
#include "core/buffers/Hash.hpp"

namespace smsc {
namespace sms {

class IllFormedReceiptParser
{
private:
    enum MsgState {
            ENROUTE = 1,
            DELIVERED,
            EXPIRED,
            DELETED,
            UNDELIVERABLE,
            ACCEPTED,
            UNKNOWN,
            REJECTED     
    };

public:
    IllFormedReceiptParser() {
        smeAckRx_.Compile("/id:(\\w+)\\s+sub:\\d+\\s+dlvrd:\\d+\\s+submit date:\\d{10} done date:\\d{10}\\s+stat:(\\w+)\\s+err:(\\d+)\\s+[tT]ext:(.*)/");
        if (smeAckRx_.LastError()!=smsc::util::regexp::errNone) throw smsc::util::Exception("regexp compilation failed");
        stateHash_["ENROUTE"] = ENROUTE;
        stateHash_["DELIVRD"] = DELIVERED;
        stateHash_["EXPIRED"] = EXPIRED;
        stateHash_["DELETED"] = DELETED;
        stateHash_["UNDELIV"] = UNDELIVERABLE;
        stateHash_["ACCEPTD"] = ACCEPTED;
        stateHash_["UNKNOWN"] = UNKNOWN;
        stateHash_["REJECTD"] = REJECTED;
    }
    /// NOTE: buf must be large enough to keep messageId (65 bytes, at least)
    const char* parseSms( SMS& sms,
                          char* buf,
                          uint8_t& message_state,
                          int& err ) {
        if (sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID)) {
            const char* res = sms.getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str();
            if (res && res[0]) {
                if (sms.hasIntProperty(Tag::SMPP_MSG_STATE)) {
                    message_state = sms.getIntProperty(Tag::SMPP_MSG_STATE);
                } else {
                    message_state = UNKNOWN;
                }
                if (sms.hasIntProperty(Tag::SMPP_NETWORK_ERROR_CODE)) {
                    err = sms.getIntProperty(Tag::SMPP_NETWORK_ERROR_CODE);
                } else {
                    err = 0;
                }
                return res;
            }
        }
        // parsing message body
        // NOTE: the code is gratefully stolen from src/system/state_machine.cpp
        std::string txt;
        smsc::util::getSmsText(&sms,txt,CONV_ENCODING_CP1251);
        smsc::util::regexp::SMatch m[10];
        int n = 10;
        if ( smeAckRx_.Match(txt.c_str(),txt.c_str()+txt.length(),m,n) ) {
            size_t idlen = m[1].end - m[1].start;
            if (idlen > 64) idlen = 64;
            memcpy(buf,txt.c_str()+m[1].start,idlen);
            buf[idlen] = '\0';
            const std::string st = txt.substr(m[2].start,m[2].end-m[2].start);
            int* p = stateHash_.GetPtr(st.c_str());
            if (p) message_state = *p;
            else message_state = UNKNOWN;
            sscanf(txt.c_str()+m[3].start,"%d",&err);
            return buf;
        }
        return 0;
    }

private:
    smsc::util::regexp::RegExp     smeAckRx_;
    smsc::core::buffers::Hash<int> stateHash_;
};

}
}

#endif
