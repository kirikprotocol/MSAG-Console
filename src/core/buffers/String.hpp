/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Yet another C++ string implementation.
  Key feature - this string can hold zero characters.
  Feature that can be considered as benefit in some cases and
  as drawback in other - quantified memory allocation.
*/

#ifndef __STRING_HPP__
#define __STRING_HPP__

#ifndef __cplusplus
#error This header for C++ only
#endif

namespace smsc{
namespace core{
namespace buffers{

//! use by String::Align
enum AlignPosition{
  apLeft,apCenter,apRight,apWords,
};


/*! String class
*/

class String{
protected:
  char *_str;
  int  _length;
  int  _size;
  int _nocase;

  void SetSize(int);
  int _Index(const char *substr,int substrlength,int pos)const;
  int _RIndex(const char *substr,int substrlength,int pos)const;
public:
  //! Empty string constructor
  String();
  //! Copy constructor
  String(const String& src);
  //! Constructor from ASCIIZ string
  String(const char* src);
  /*! This constructor will preallocate {num} bytes
      \param num - amount of bytes to preallocate
  */
  explicit String(int num);
  //! Destructor :)
  virtual ~String();

  //! assignment operator for String
  void operator=(const String& src);
  //! assignment operator for ASCIIZ string
  void operator=(const char* src);
  //! assignment operator for single char
  void operator=(char src);


  int operator==(const String& cmp)const;
  int operator==(const char* cmp)const;

  int operator!=(const String& cmp)const;
  int operator!=(const char* cmp)const;

  int operator<(const String& cmp)const;
  int operator<(const char* cmp)const;

  int operator>(const String& cmp)const;
  int operator>(const char* cmp)const;

  int operator<=(const String& cmp)const;
  int operator<=(const char* cmp)const;

  int operator>=(const String& cmp)const;
  int operator>=(const char* cmp)const;

  String operator+(const String& src)const;
  String operator+(const char* src)const;
  String operator+(const char src)const;
  /*! This will add numeric representation of integer
    \param src - integer number that will be added to string
  */
  String operator+(int src)const;

  /*!
    Operator that will enable expressions like this:<br>
    str="hello"+str2;
  */
  friend String operator+(const char* str1,const String& str2)
  {
    String tmp(str1);
    tmp+=str2;
    return tmp;
  }

  void operator+=(const String& src);
  void operator+=(const char* src);
  void operator+=(const char src);
  //! This will append numeric representation of integer
  void operator+=(int src);

  //! The same as +=
  String& operator<<(const String& src);
  //! The same as +=
  String& operator<<(const char* src);
  //! The same as +=
  String& operator<<(char src);
  //! The same as +=
  String& operator<<(int src);

  //! char* typecast operator
  operator const char*(){SetSize(0);return _str;}
  //! constant version of char* typecast operator
  operator const char*()const{return _str;}

  /*! single char indexing operator
    \param index - index of char to retrieve
    if index < 0 that chars from the end of string are indexed
    i.e. char with index -1 is last char of string,
    with index -2 is the one before the last, etc
  */
  char& operator[](int index)
  {
    if(index<0)index+=_length;
    if(index>_length)index=_length-1;
    return _str[index];
  }

  //! constant version of char& operator[](int index)
  const char operator[](int index)const
  {
    if(index<0)index+=_length;
    if(index>_length)index=_length-1;
    return _str[index];
  }

  //! Will return ASCIIZ pointer to string
  const char* Str()const{return _str;}

  //! Return string concatenated with self num times
  String operator*(int num);
  //! Selfconcatenate string num times
  void operator*=(int num);

  /*! Reallocate string buffer, preserving content
      \param newsize - new size of string buffer.
  */
  int Resize(int newsize);
  //! return length of the string
  int Length()const{return _length;}
  //! Shrink or enlarge string to the length - newlength
  void SetLength(int newlength);
  /*! Set 'ignorecase' mode
      \param newnocase - new mode. 0 - disable, 1 - enable
      \return previous mode
  */
  int SetNoCase(int newnocase){int oldcase=_nocase;_nocase=newnocase;return oldcase;}
  //! Compare string ignoring case with ASCIIZ string
  int CmpNoCase(const char * cmp)const;

  /*! Get substring of the string
      \param index - offset of the first char of new string
      \param count - number of chars to get, or -1 if need substring
      from the index till the end.
      \return new string/
  */
  String Substr(int index,int count=-1)const;

  /*! Set string content from the ASCIIZ string
      \param src - source ASCIIZ string
      \param index - offset of the first char
      \param count - number of chars to copy
  */
  void Set(const char* src,int index=0,int count=-1);
  /*! Concatenate string with part of ASCIIZ string
      \param src - source ASCIIZ string
      \param index - offset of first char to concatenate
      \param count - number of chars to concatenate
  */
  void Concat(const char* src,int index=0,int count=-1);

  /*! Find substring
      \param substr - substring to search
      \param pos - offset where to start search
      \return - offset of substring if found, or -1 otherwise
  */
  int Index(const String& substr,int pos=0)const;
  //! Same as int Index(const String& substr,int pos=0)const;, but for ASCIIZ string
  int Index(const char* substr,int pos=0)const;
  //! Same as int Index(const String& substr,int pos=0)const;, but search from the right
  int RIndex(const String& substr,int pos=-1)const;
  //! Same as int RIndex(const String& substr,int pos=-1)const;, but for ASCIIZ string
  int RIndex(const char* substr,int pos=-1)const;

  /*! Insert string
      \param index - offset where to insert string. if index<0 offset
      calculated from the end of the string
      \param ins - string to insert
  */
  void Insert(int index,const String& ins);
  //! Same as void Insert(int index,const String& ins);, but for ASCIIZ string
  void Insert(int index,const char* ins);

  /*! Delete substring
      \param index - offset of first char to delete. If index <0 then
      chars counted from the end of string.
      \param count - number of chars to delete, or -1 to delete till the end
  */
  void Delete(int index,int count=-1);

  /*! Replace substring
      \param src - substring that will be replaced
      \param dst - substring will replace src
  */
  int Replace(const String& src,const String& dst);
  //! Same as void Replace(const String& src,const String& dst);, but for ASCIIZ src
  int Replace(const char* src,const char* dst);

  /*! Replace chars
    \param from - what char to replace
    \param to - to which char to replace
  */
  int Replace(char from,char to);

  /*! Fill string with char
      \param src - char that will fill the string
      \param num - new length of the string
  */
  void Fill(char src,int num);

  /*! Format string
      \sa sprintf from libc
  */
  int Sprintf(const char *fmt,...);

  /*! Append formated string
      \sa sprintf from libc
  */
  int AppendSprintf(const char *fmt,...);

  //! Free memory allocated by string
  void Clean();

  //! Delete trailing spaces
  void RTrim();
  //! Delete leading spaces
  void LTrim();
  //! Delete leading and trailing spaces
  void Trim(){RTrim();LTrim();}
  /*! Align string
      \param where - if set to apRight, fill with spaces to left till new size
      if set to apCenter, fill with spaces from the left and right equally till the new size,
      if set to apLeft, fill with spaces from the left till the new size,
      if set to apWords, try to fill with spaces between words equally, till the new size.
      \param size - new size of string (with spaces).

  */
  void Align(int where,int size);

  String& ToLower();
  String& ToUpper();

  int ToInt()const;


  //! Load string from file
  int LoadFromFile(const char* filename);
};

};//buffers
};//core
};//smsc

#endif
