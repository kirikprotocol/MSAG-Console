#ifndef _SCAG_COUNTER_IMPL_CONFIGREADER_H
#define _SCAG_COUNTER_IMPL_CONFIGREADER_H

#include <map>
#include <string>
#include "scag/config/base/ConfigView.h"
#include "scag/counter/ActionTable.h"

namespace scag2 {
namespace counter {

class TemplateManager;

namespace impl {

class ConfigReader
{
    struct TemplateProto {
        std::string typeName;
        std::string limitName;
        int64_t     param0, param1;
    };

    typedef std::map<std::string,ActionList>    LimitMap;
    typedef std::map<std::string,TemplateProto> ProtoMap;

public:
    void reloadConfig( TemplateManager& tmgr,
                       const config::ConfigView& view );
    
private:
    /// @param view is a section with counter limits.
    void reloadLimits( config::ConfigView& view );
    /// @param view is a section with templates.
    void reloadTemplates( TemplateManager& tmgr,
                          config::ConfigView& view );
private:

};

}
}
}

#endif /* !_SCAG_COUNTER_IMPL_CONFIGREADER_H */
