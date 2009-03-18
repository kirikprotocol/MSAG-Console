#ifndef __EYELINE_UTILX_THREADSPECIFICDATA_HPP__
# define __EYELINE_UTILX_THREADSPECIFICDATA_HPP__

# include <pthread.h>

namespace eyeline {
namespace utilx {

extern "C"
static void _destroy(void *ptr) {
  delete ptr;
}

template<class T>
class ThreadSpecificData {
public:
  ThreadSpecificData() {
    int ret = pthread_key_create(&_tsd, _destroy);
    if ( ret )
      throw smsc::util::SystemError("ThreadSpecificData::ThreadSpecificData::: call to pthread_key_create() failed");
  }
  ~ThreadSpecificData() {
    pthread_key_delete(_tsd);
  }

  void set(const T& value) {
    T* tsdValue = static_cast<T*>(pthread_getspecific(_tsd));
    if ( !tsdValue ) {
      tsdValue = new T(value);
      pthread_setspecific(_tsd, tsdValue);
    } else
      *tsdValue = value;
  }

  virtual T& get() {
    T* tsdValue = static_cast<T*>(pthread_getspecific(_tsd));
    if ( !tsdValue )
      throw smsc::util::Exception("ThreadSpecificData::get::: value wasn't set");
    else
      return *tsdValue;
  }

  virtual const T& get(const T& defaultValue) const {
    T* tsdValue = static_cast<T*>(pthread_getspecific(_tsd));

    if ( !tsdValue )
      return defaultValue;
    else
      return *tsdValue;
  }
private:
  pthread_key_t _tsd;
};

}}

#endif
