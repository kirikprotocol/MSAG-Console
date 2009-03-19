#include "SuaApiFactory.hpp"
#include "SuaUser.hpp"
#include <util/Exception.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

SuaApi*
SuaApiFactory::_instance;

void
SuaApiFactory::init()
{
  if ( !_instance ) _instance = new SuaUser();
}

SuaApi&
SuaApiFactory::getSuaApiIface()
{
  if ( !_instance ) throw smsc::util::Exception("SuaApiFactory::getSuaApiIface::: SuaApi implemetation wasn't initialized");
  return *_instance;
}

}}}
