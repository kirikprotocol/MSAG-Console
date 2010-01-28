#include <memory>
#include "ConfigReader.h"

namespace scag2 {
namespace counter {
namespace impl {

using namespace scag2::config;

void ConfigReader::reloadConfig( TemplateManager&    tmgr,
                                 const ConfigView& view )
{
    std::auto_ptr<CStrSet> sections(view.getShortSectionNames());
    if (!sections.get())
        throw ConfigException("cannot read configlimits section names");
    for ( CStrSet::iterator i = sections->begin(); i != sections->end(); ++i ) {
        std::auto_ptr<ConfigView> subc(view.getSubConfig(i->c_str()));
        if (!subc.get()) throw ConfigException("cannot get subconfig %s",i->c_str());
        
    }
}

}
}
}
