#ifndef BillingManagerConfig_dot_h
#define BillingManagerConfig_dot_h

#include "scag/config/ConfigView.h"
//#include "scag/re/actions/ActionFactory.h"

namespace scag { namespace re { namespace actions {
     class ActionFactory;
}}}

namespace scag {
namespace config {

using scag::re::actions::ActionFactory;


class BillingManagerConfig
{
public:
    std::string cfg_dir;
    std::string so_dir;
    scag::re::actions::ActionFactory * mainActionFactory;
    BillingManagerConfig() : cfg_dir(""), so_dir(""),mainActionFactory(0) { };
    BillingManagerConfig(ConfigView& cv);
    void init(ConfigView& cv);   
    bool check(ConfigView& cv);
};

}
}

#endif
