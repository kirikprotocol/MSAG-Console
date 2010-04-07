#ifndef __EYELINE_SS7NA_COMMON_KEYSREGISTRY_HPP__
# define __EYELINE_SS7NA_COMMON_KEYSREGISTRY_HPP__

# include <set>

# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

template <class KEY>
class KeysRegistry {
public:
  void insert(const KEY& key) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    _knownKeys.insert(key);
  }

  void remove(const KEY& key) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    _knownKeys.erase(key);
  }

  bool hasElement(const KEY& key) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    return _knownKeys.find(key) != _knownKeys.end();
  }
private:
  smsc::core::synchronization::Mutex _lock;
  std::set<KEY> _knownKeys;
};

}}}

#endif
