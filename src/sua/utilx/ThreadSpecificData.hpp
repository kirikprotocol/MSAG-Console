#ifndef __SUA_UTILX_THREADSPECIFICDATA_HPP__
# define __SUA_UTILX_THREADSPECIFICDATA_HPP__

# include <pthread.h>

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
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("tsd");
    smsc_log_debug(logger, "ThreadSpecificData::set::: set value=[%d]", value);
    T* tsdValue = static_cast<T*>(pthread_getspecific(_tsd));
    if ( !tsdValue ) {
      tsdValue = new T(value);
      pthread_setspecific(_tsd, tsdValue);
    } else
      *tsdValue = value;
  }

  virtual T& get() {
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("tsd");
    smsc_log_debug(logger, "ThreadSpecificData::set::: called get()");
    T* tsdValue = static_cast<T*>(pthread_getspecific(_tsd));
    if ( !tsdValue )
      throw smsc::util::Exception("ThreadSpecificData::get::: value wasn't set");
    else
      return *tsdValue;
  }

  virtual const T& get(const T& defaultValue) const {
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("tsd");
    smsc_log_debug(logger, "ThreadSpecificData::get::: called get(defaultValue=%d)", defaultValue);

    T* tsdValue = static_cast<T*>(pthread_getspecific(_tsd));
    smsc_log_debug(logger, "ThreadSpecificData::get::: tsdValue=%p, defaultValue=%d", tsdValue, defaultValue);
    if ( !tsdValue )
      return defaultValue;
    else
      return *tsdValue;
  }
private:
  pthread_key_t _tsd;
};

}

#endif
