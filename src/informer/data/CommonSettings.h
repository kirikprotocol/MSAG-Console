#ifndef _INFORMER_COMMONSETTINGS_H
#define _INFORMER_COMMONSETTINGS_H

#include <string>

namespace eyeline {
namespace informer {

class CommonSettings
{
public:
    CommonSettings() : path_("./") {}

    void init( const std::string& path ) {
        path_ = path;
        if (path_.empty()) path_ = "./";
        else if (path_[path_.size()-1] != '/') path_.push_back('/');
    }

    inline const std::string& getStorePath() const {
        return path_;
    }

private:
    std::string path_;
};

} // informer
} // smsc

#endif
