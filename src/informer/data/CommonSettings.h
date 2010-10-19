#ifndef _INFORMER_COMMONSETTINGS_H
#define _INFORMER_COMMONSETTINGS_H

#include <string>

namespace eyeline {
namespace informer {

class CommonSettings
{
public:
    CommonSettings() : path_("./"),
    unrespondedMessagesMax_(100),
    responseWaitTime_(65),
    receiptWaitTime_(600),
    svcType_("ESME"),
    protocolId_(1)   // FIXME
    {}

    void init( const std::string& path ) {
        path_ = path;
        if (path_.empty()) path_ = "./";
        else if (path_[path_.size()-1] != '/') path_.push_back('/');
    }

    inline const std::string& getStorePath() const {
        return path_;
    }

    // maximum number of unresponded messages per SMSC
    inline unsigned getUnrespondedMessagesMax() const { return unrespondedMessagesMax_; }

    inline unsigned getResponseWaitTime() const { return responseWaitTime_; }

    inline unsigned getReceiptWaitTime() const { return receiptWaitTime_; }

    const char* getSvcType() const { return svcType_.c_str(); }

    unsigned getProtocolId() const { return protocolId_; }

    unsigned getMaxMessageChunkSize() const { return 160; /* FIXME */ }

private:
    std::string path_;
    unsigned unrespondedMessagesMax_;
    unsigned responseWaitTime_;
    unsigned receiptWaitTime_;
    std::string svcType_;
    unsigned protocolId_;
};

} // informer
} // smsc

#endif
