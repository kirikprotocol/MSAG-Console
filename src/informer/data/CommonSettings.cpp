#include "CommonSettings.h"
#include "informer/io/UTF8.h"

namespace eyeline {
namespace informer {

CommonSettings::CommonSettings() :
path_("./"),
statpath_("./"),
unrespondedMessagesMax_(100),
responseWaitTime_(65),
receiptWaitTime_(600),
svcType_("ESME"),
protocolId_(1), // FIXME
incStatBank_(0),
utf8_(0)
{
}


CommonSettings::~CommonSettings()
{
    delete utf8_;
}


void CommonSettings::init( const std::string& path, const std::string& statpath )
{
    utf8_ = new UTF8();

    path_ = path;
    if (path_.empty()) path_ = "./";
    else if (path_[path_.size()-1] != '/') path_.push_back('/');
    statpath_ = statpath;
    if (statpath_.empty()) statpath_ = "./";
    else if (statpath_[statpath_.size()-1] != '/') statpath_.push_back('/');
}

} // informer
} // smsc
