#include "MSAGLinkSet.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace balancers {

io_subsystem::LinkId
MSAGLinkSet::send(const io_subsystem::Message& message)
{
  throw smsc::util::Exception("MSAGLinkSet::send::: not implemented yet");
}

}}}
