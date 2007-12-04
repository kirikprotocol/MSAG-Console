#ifndef __SMSC_UTIL_OBJECTREGISTRY_HPP__
# define __SMSC_UTIL_OBJECTREGISTRY_HPP__ 1

# include <util/Singleton.hpp>
# include <utility>
# include <map>

# include <logger/Logger.h>

namespace smsc {
namespace util {

template <class OBJECT, class CRITERION>
class ObjectRegistry : public smsc::util::Singleton<ObjectRegistry<OBJECT,CRITERION> >
{
public:
  OBJECT* getObject(const CRITERION& searchCriterion) {
    objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() ) return iter->second;
    else return NULL;
  }
  
  void toRegisterObject(OBJECT* newObject, const CRITERION& searchCriterion) {
    objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter == _objectsRegistryMap.end() )
      _objectsRegistryMap.insert(std::make_pair(searchCriterion, newObject));
  }

  OBJECT* toUnregisterObject(const CRITERION& searchCriterion) {
    objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() ) {
      OBJECT* obj = iter->second;
      _objectsRegistryMap.erase(iter);
      return obj;
    } else return NULL;
  }
private:
  typedef std::map<CRITERION,OBJECT*> objectRegistryMap_t;
  objectRegistryMap_t _objectsRegistryMap;
};

template <class OBJECT, class CRITERION>
class RefObjectRegistry : public smsc::util::Singleton<RefObjectRegistry<OBJECT,CRITERION> >
{
public:
  smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex>
  getObject(const CRITERION& searchCriterion) {
    objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    smsc::logger::Logger* logger(smsc::logger::Logger::getInstance("dbStrgCon"));
    if ( iter != _objectsRegistryMap.end() )
      return iter->second;
    else
      return smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex>();
  }
  
  void toRegisterObject(smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex>& newObject, const CRITERION& searchCriterion) {
    objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter == _objectsRegistryMap.end() )
      _objectsRegistryMap.insert(std::make_pair(searchCriterion, newObject));
  }

  smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex>
  toUnregisterObject(const CRITERION& searchCriterion) {
    objectRegistryMap_t::iterator iter = _objectsRegistryMap.find(searchCriterion);
    if ( iter != _objectsRegistryMap.end() ) {
      smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex> obj = iter->second;
      _objectsRegistryMap.erase(iter);
      return obj;
    } else
      return smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex>();
  }
private:
  typedef std::map<CRITERION,smsc::core::buffers::RefPtr<OBJECT, smsc::core::synchronization::Mutex> > objectRegistryMap_t;
  objectRegistryMap_t _objectsRegistryMap;
};

}
}

#endif
