#ifndef _SCAG_CONFIG_TESTCONFIG_H
#define _SCAG_CONFIG_TESTCONFIG_H

#include <memory>
#include "Config.h"

namespace scag {
namespace config {

/// Class for simple tests which requres config functionality
/// but don't want to pull all that stuff that is linked to ConfigManager.
class TestConfig
{
public:
    TestConfig() {}
    ~TestConfig() {}
    void init() throw (ConfigException);
    const Config* getConfig() const { return cfg_.get(); }

private:
    std::auto_ptr< Config > cfg_;
};

} // namespace config
} // namespace scag

#endif /* !_SCAG_CONFIG_TESTCONFIG_H */
