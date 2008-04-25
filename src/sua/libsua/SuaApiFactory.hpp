#ifndef __SUA_LIBSUA_SUAAPIFACTORY_HPP__
# define __SUA_LIBSUA_SUAAPIFACTORY_HPP__ 1

# include <sua/libsua/SuaApi.hpp>

namespace libsua {

class SuaApiFactory {
public:
  static void init();
  static SuaApi& getSuaApiIface();
private:
  static SuaApi* _instance;
};

}

#endif
