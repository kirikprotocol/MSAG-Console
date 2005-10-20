#include "route_manager.h"
#include "scag/config/route/RouteConfig.h"

namespace scag{
namespace transport{
namespace smpp{
namespace router{

extern void loadRoutes(RouteManager* rm,const scag::config::RouteConfig& rc,bool traceit=false);


}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router
