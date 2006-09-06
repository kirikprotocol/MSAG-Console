#ifndef __SMSC_UTIL_SINGLETON_HPP__
# define __SMSC_UTIL_SINGLETON_HPP__ 1

# include <util/Exception.hpp>

namespace smsc {
namespace util {

template <class T>
class Singleton {
public:
  static T& getInstance() {
    static T _instance;
    return _instance;
  }
};

}
}

#endif
