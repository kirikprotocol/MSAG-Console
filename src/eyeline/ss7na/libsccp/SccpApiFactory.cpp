#include "SccpApiFactory.hpp"
#include "SccpUser.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

SccpApi*
SccpApiFactory::_instance;

void
SccpApiFactory::init()
{
  if ( !_instance ) _instance = new SccpUser();
}

SccpApi&
SccpApiFactory::getSccpApiIface()
{
  if ( !_instance ) throw smsc::util::Exception("SccpApiFactory::getSccpApiIface::: SccpApi implementation wasn't initialized");
  return *_instance;
}

}}}
