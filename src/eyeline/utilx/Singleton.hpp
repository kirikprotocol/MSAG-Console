#ifndef __EYELINE_UTILX_SINGLETON_HPP__
# define __EYELINE_UTILX_SINGLETON_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace utilx {

template <class T>
class Singleton {
public:
  static void init() { if ( !_instance ) _instance = new T(); }
  static void deinit() { delete _instance; }

  static T& getInstance() {
    if ( !_instance ) throw smsc::util::Exception("Singleton<T>::getInstance::: singleton wasn't initialized");
    return *_instance;
  }
private:
  static T* _instance;
};

template <class T>
T* Singleton<T>::_instance;

template <class T>
class Singleton<T*> {
public:
  static void init() { if ( !_instance ) _instance = new T(); }
  static void deinit() { delete _instance; _instance = NULL; }

  static T*& getInstance() {
    if ( !_instance ) throw smsc::util::Exception("Singleton<T*>::getInstance::: singleton wasn't initialized");
    return _instance;
  }
private:
  static T* _instance;
};

template <class T>
T* Singleton<T*>::_instance;

}}

#endif
