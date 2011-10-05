#ifndef SMSC_UTIL_AUTO_ARR_PTR_HPP
#define SMSC_UTIL_AUTO_ARR_PTR_HPP

namespace smsc{
namespace util{

template <class TArg /* : public UtilizableObjITF */>
class auto_arr_ptr{
private:
  TArg * m_ptr;

public:
  explicit auto_arr_ptr(TArg * use_ptr = 0) throw()
  : m_ptr(use_ptr)
  { }
  auto_arr_ptr(auto_arr_ptr& use_org) throw()
            : m_ptr(use_org.release())
  { }
  ~auto_arr_ptr()
  {
    reset(0);
  }

  TArg * get() throw()
  {
    return m_ptr;
  }
  TArg * get() const throw()
  {
    return m_ptr;
  }
  TArg * release() throw()
  {
    TArg * pt = m_ptr;
    m_ptr = 0;
    return pt;
  }
  void reset(TArg * use_ptr = 0) //may throw !!!
  {
    if (m_ptr != use_ptr)
    {
      TArg * pt = m_ptr;
      m_ptr = use_ptr;
      delete [] pt;
    }
  }

  TArg * operator->() throw()
  {
    return get();
  }
  const TArg * operator->() const throw()
  {
    return get();
  }
  auto_arr_ptr & operator=(auto_arr_ptr & use_org) //may throw !!!
  {
    reset(use_org.release());
    return *this;
  }
};

}
}


#endif
