#ifndef __SUA_UTILX_ITERATOR_HPP__
# define __SUA_UTILX_ITERATOR_HPP__ 1

namespace utilx {

template <class T>
class Iterator {
public:
  virtual bool hasElement() const = 0;
  virtual void next() = 0;

  typedef const T& const_ref;
  virtual const_ref getCurrentElement() const = 0;

  typedef T& ref;
  virtual ref getCurrentElement() = 0;

  virtual void deleteCurrentElement() = 0;
};

} // namespace

#endif
