/* ************************************************************************** *
 * Object Registry templates.
 * ************************************************************************** */
#ifndef __ELC_UTIL_POBJ_REGISTRY_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ELC_UTIL_POBJ_REGISTRY_HPP__

#include <map>

namespace eyeline {
namespace util {

//Pointered objects registry. Objects are deleted upon registry destruction.
template <class _KeyTArg, class _TArg>
class POBJRegistry_T {
protected:
  typedef std::map<_KeyTArg, _TArg *> TRegistry;
  TRegistry  registry;

public:
  typedef typename TRegistry::value_type      value_type;
  typedef typename TRegistry::size_type       size_type;

  POBJRegistry_T()
  { }
  ~POBJRegistry_T()
  {
    for (typename TRegistry::iterator sit = registry.begin(); sit != registry.end(); ++sit)
      delete sit->second;
  }

  class const_iterator {
  protected:
    friend class POBJRegistry_T;
    typename TRegistry::const_iterator rIt;

    const_iterator(const typename TRegistry::const_iterator & use_it)
      : rIt(use_it)
    { }

  public:
    const_iterator()
    { }
    const_iterator(const const_iterator & use_it) : rIt(use_it.rIt)
    { }
    ~const_iterator()
    { }

    const_iterator & operator=(const const_iterator & use_it) { rIt = use_it.rIt; }
    const value_type & operator*() const { return *rIt; }
    const value_type * operator->() const { return rIt.operator->(); }

    const_iterator & operator++() //preincrement
    {
      ++rIt; return *this;
    }
    const_iterator  operator++(int) //postincrement
    {
      const_iterator  _tmp = *this;
      ++rIt; return _tmp;
    }
    const_iterator & operator--()  //predecrement
    {
      --rIt; return *this;
    }
    const_iterator  operator--(int)    //postdecrement
    {
      const_iterator  _tmp = *this;
      --rIt; return _tmp;
    }
    bool operator==(const const_iterator & i2) const
    {
      return (rIt == i2.rIt);
    }
    bool operator!=(const const_iterator & i2) const
    {
      return (rIt != i2.rIt);
    }
  };

  const_iterator begin(void) const { return const_iterator(registry.begin()); }
  const_iterator end(void) const { return const_iterator(registry.end()); }

  size_type size(void) const { return registry.size(); }
  bool empty(void) const { return registry.empty(); }


  _TArg * find(const _KeyTArg & use_key) const
  {
    typename TRegistry::const_iterator it = registry.find(use_key);
    return (it != registry.end()) ? it->second : NULL;
  }

  bool insert(const _KeyTArg & use_key, _TArg * p_obj)
  {
    std::pair<typename TRegistry::iterator, bool> res =
      registry.insert(typename TRegistry::value_type(use_key, p_obj));
    return res.second;
  }

  void reset(const _KeyTArg & use_key, _TArg * p_obj)
  {
    typename TRegistry::iterator it = registry.find(use_key);
    if (it != registry.end()) {
      delete it->second;
      it->second = p_obj;
    } else
      insert(use_key, p_obj);
  }

  bool erase(const _KeyTArg & use_key)
  {
    typename TRegistry::iterator it = registry.find(use_key);
    if (it != registry.end()) {
      delete it->second;
      registry.erase(it);
      return true;
    }
    return false;
  }
};

} //util
} //eyeline

#endif /* __ELC_UTIL_POBJ_REGISTRY_HPP__ */

