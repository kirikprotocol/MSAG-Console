#ifndef __EYELINE_UTILX_OBJECTREGISTRY_HPP__
# define __EYELINE_UTILX_OBJECTREGISTRY_HPP__

# include <map>
# include <utility>

# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/utilx/FakeMutex.hpp"
# include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace utilx {

template <class OBJECT, class CRITERION,
          class LOCK = FakeMutex>
class ObjectRegistry {
public:
  bool exists (const CRITERION& searchCriterion) const {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    if ( _objectsRegistryMap.find(searchCriterion) == _objectsRegistryMap.end() )
      return false;
    else
      return true;
  }

  OBJECT lookup(const CRITERION& searchCriterion) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    typename objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() ) return iter->second;
    else throw utilx::RegistryKeyNotFound("ObjectRegistry::lookup::: key not found");
  }

  void insert(const OBJECT& newObject, const CRITERION& searchCriterion) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    typename objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter == _objectsRegistryMap.end() )
      _objectsRegistryMap.insert(std::make_pair(searchCriterion, newObject));
  }

  void remove(const CRITERION& searchCriterion) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    typename objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() )
      _objectsRegistryMap.erase(iter);
  }

private:
  typedef std::map<CRITERION,OBJECT> objectRegistryMap_t;
  objectRegistryMap_t _objectsRegistryMap;
  mutable LOCK _lock;
};

template <class OBJECT, class CRITERION,
          class LOCK = FakeMutex>
class RefObjectRegistry {
public:
  bool exists (const CRITERION& searchCriterion) const {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    if ( _objectsRegistryMap.find(searchCriterion) == _objectsRegistryMap.end() )
      return false;
    else
      return true;
  }

  smsc::core::buffers::RefPtr<OBJECT, LOCK>
  lookup(const CRITERION& searchCriterion) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    typename objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() )
      return iter->second;
    else
      return smsc::core::buffers::RefPtr<OBJECT, LOCK>();
  }
  
  void insert(const smsc::core::buffers::RefPtr<OBJECT, LOCK>& newObject,
              const CRITERION& searchCriterion) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    typename objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter == _objectsRegistryMap.end() )
      _objectsRegistryMap.insert(std::make_pair(searchCriterion, newObject));
  }

  smsc::core::buffers::RefPtr<OBJECT, LOCK>
  remove(const CRITERION& searchCriterion) {
    smsc::core::synchronization::MutexGuard synchronize(_lock);
    typename objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() ) {
      smsc::core::buffers::RefPtr<OBJECT, LOCK> obj = iter->second;
      _objectsRegistryMap.erase(iter);
      return obj;
    } else
      return smsc::core::buffers::RefPtr<OBJECT, LOCK>();
  }

private:
  typedef std::map<CRITERION, smsc::core::buffers::RefPtr<OBJECT, LOCK> > objectRegistryMap_t;
  objectRegistryMap_t _objectsRegistryMap;
  LOCK _lock;
};

}}

#endif
