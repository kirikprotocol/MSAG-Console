/*
  $Id$
  Copyright (C) 2002, Alexey Chen (hedgehog@molestudio.com)
  http://www.molestudio.com/
*/


#if !defined (HEADER_A_PTR_H)
#define HEADER_A_PTR_H
#pragma warning(disable:4284)
/**
  \class a_ptr
  \brief Сторож динамически распределенной памяти.
  \note При разрушении вазывается деструктор для динамически распределенного обьекта.
*/
template<class T> class a_ptr{
public:
  a_ptr(T* p=0) : ptr(p) {}
  a_ptr(const a_ptr& p) {ptr = p.forget();}
  ~a_ptr() { if (ptr) delete ptr; }
  T** operator&() const {return &ptr;}
  T* operator->() const {return ptr;}
  operator T*() const { return ptr; }
  void operator = (T* p) { if (ptr) delete ptr; ptr = p;}
  T* operator +(int i) const { require(ptr!=0); return ptr+i; }
  bool IsNot0() const { return ptr!=0; }
  bool IsNull() const { return ptr==0; }
  T* get() const {return t;}
  T* forget() const { T* tmp = ptr; ptr=0; return tmp; }
private:
  mutable T* ptr;
  void operator = (const a_ptr&) {}
};

/**
  \class a_ptr_arr
  \brief Сторож динамически распределенного массива обьектов.
  \note При разрушении области памяти вызывается деструктор для каждого элемента массива.
*/
template<class T> class a_ptr_arr{
public:
  a_ptr_arr(T* p=0) : arr(p) {}
  a_ptr_arr(const a_ptr_arr& p) { arr = p.forget();}
  ~a_ptr_arr() { if (arr) delete[] arr; }
  T** operator&() const {return &arr;}
  T* operator->() const {return arr;}
  operator void*() const { return (void*)arr; }
  void operator = (T* p) { if (arr) delete[] arr; arr = p;}
  T& operator [](int i) const { require(arr!=0); return arr[i]; }
  T* operator +(int i) const { require(arr!=0); return arr+i; }
  bool IsNot0() const { return arr!=0; }
  bool IsNull() const { return arr==0; }
  T* get() const {return arr;}
  T* forget() const { T* tmp = arr; arr=0; return tmp; }
private:
  mutable T* arr;
  void operator = (const a_ptr_arr&) {}
};
#pragma warning(default:4284)

#endif //HEADER_A_PTR_H
