#ifndef __EYELINE_SUA_LIBSUA_SUAAPIFACTORY_HPP__
# define __EYELINE_SUA_LIBSUA_SUAAPIFACTORY_HPP__

# include <eyeline/sua/libsua/SuaApi.hpp>

namespace eyeline {
namespace sua {
namespace libsua {

class SuaApiFactory {
public:
  static void init();
  static SuaApi& getSuaApiIface();
private:
  static SuaApi* _instance;
};

}}}

#endif
