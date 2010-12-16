#include "route_manager.h"
#include "scag/config/route/RouteConfig.h"

namespace scag{
namespace transport{
namespace smpp{
namespace router{

extern void loadRoutes(RouteManager* rm,
                       const scag::config::RouteConfig& rc,
                       std::vector< std::string >* traceit = 0);

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router
