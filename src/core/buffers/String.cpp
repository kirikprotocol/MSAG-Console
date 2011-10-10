/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2000-2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author!

  Yet another C++ string implementation.
  Key feature - this string can hold zero characters.
  Feature that can be considered as benefit in some cases and
  as drawback is other - quantified memory allocation.

*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "String.hpp"

namespace smsc{
namespace core{
namespace buffers{


static inline void xmemcpy(void* dst,const void* src,int size)
{
  if(!dst || !src)return;
  memcpy(dst,src,size);
}

String::String()
{
  _str=0;
  _length=0;
  _size=0;
  _nocase=0;
}

String::String(const String& src)
{
  _str=0;
  _length=0;
  _size=0;
  _nocase=0;
  SetSize(src.Length());
  xmemcpy(_str,(const char*)src,src.Length()+1);
  _length=src.Length();
}

String::String(const char* src)
{
  _nocase=0;
  _length=(int)strlen(src);
  _size=_length+16;
  _str=new char[_size];
  xmemcpy(_str,src,_length+1);
}

String::String(int num)
{
  _nocase=0;
  _length=0;
  _str=new char[num+1];
  _str[0]=0;
  _size=num+1;
}

String::~String()
{
  delete [] _str;
  _str=0;
  _length=0;
  _size=0;
}


void String::SetSize(int newsize)
{
  if(newsize<_size)return;
  delete [] _str;
  _size=newsize+newsize/4+1;
  _str=new char[_size];
  _length=0;
  _str[0]=0;
}

int String::Resize(int newsize)
{
  if(newsize<_size)return 0;
  char *tmpstr=new char[newsize+newsize/4+1];
  if(!tmpstr)return 0;
  if(_str)xmemcpy(tmpstr,_str,_length);
  delete [] _str;
  _str=tmpstr;
  _size=newsize+newsize/4+1;
  _str[_length]=0;
  return 1;
}

void String::operator=(const String& src)
{
  SetSize(src.Length());
  xmemcpy(_str,(const char*)src,src.Length()+1);
  _length=src.Length();
}

void String::operator=(const char* src)
{
  int len=(int)strlen(src);
  SetSize(len);
  _length=len;
  xmemcpy(_str,src,_length+1);
}

void String::operator=(char src)
{
  SetSize(4);
  _length=1;
  _str[0]=src;
  _str[1]=0;
}

/*
int String::operator==(const String& cmp)
{
  int i;
  if(_nocase)return CmpNoCase(cmp);
  if((const char*)cmp==0)return _str==0;
  if(!_str)return !(const char*)cmp;
  if(_length!=cmp.Length())return 0;
  for(i=0;i<_length;i++)
  {
    if(_str[i]!=((const char*)cmp)[i])return 0;
  }
  return 1;
}

int String::operator==(const char* cmp)
{
  int i;
  if(_nocase)return CmpNoCase(cmp);
  if(!cmp)return _str==0;
  if(!_str)return !cmp;
  for(i=0;i<_length;i++)
  {
    if(_str[i]!=cmp[i])return 0;
  }
  if(cmp[i]!=0)return 0;
  return 1;
}
*/

int String::operator==(const String& cmp)const
{
  int i;
  if(_nocase)return CmpNoCase(cmp);
  if((const char*)cmp==0)return _str==0;
  if(!_str)return !(const char*)cmp;
  if(_length!=cmp.Length())return 0;
  for(i=0;i<_length;i++)
  {
    if(_str[i]!=((const char*)cmp)[i])return 0;
  }
  return 1;
}

int String::operator==(const char* cmp)const
{
  int i;
  if(_nocase)return CmpNoCase(cmp);
  if(!cmp)return _str==0;
  if(!_str)return !cmp;
  for(i=0;i<_length;i++)
  {
    if(_str[i]!=cmp[i])return 0;
  }
  if(cmp[i]!=0)return 0;
  return 1;
}

int String::CmpNoCase(const char* cmp)const
{
  int i;
  if(!cmp)return _str==0;
  if(!_str)return !cmp;
  for(i=0;cmp[i];i++)
  {
    if(tolower(_str[i])!=tolower(cmp[i]))
    {
      return 0;
    }
  }
  if(i!=_length)return 0;
  return 1;
}

int String::operator!=(const String& cmp)const
{
  return !(*this==cmp);
}

int String::operator!=(const char* cmp)const
{
  return !(*this==cmp);
}

int String::operator<(const String& cmp)const
{
  return operator<((const char*)cmp);
}

int String::operator<(const char* cmp)const
{
  int i;
  if(_nocase)
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(tolower(_str[i])<tolower(cmp[i]))return 1;
    }
  }else
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(_str[i]<cmp[i])return 1;
    }
  }
  if(cmp[i])return 1;
  return 0;
}

int String::operator>(const String& cmp)const
{
  return operator>((const char*)cmp);
}

int String::operator>(const char* cmp)const
{
  int i;
  if(_nocase)
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(tolower(_str[i])>tolower(cmp[i]))return 1;
    }
  }else
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(_str[i]>cmp[i])return 1;
    }
  }
  if(i<_length && !cmp[i])return 1;
  return 0;
}

int String::operator<=(const String& cmp)const
{
  return operator<=((const char*)cmp);
}

int String::operator<=(const char* cmp)const
{
  int i;
  if(_nocase)
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(tolower(_str[i])>tolower(cmp[i]))return 0;
    }
  }else
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(_str[i]>cmp[i])return 0;
    }
  }
  if(!cmp[i] && i<_length)return 0;
  return 1;
}

int String::operator>=(const String& cmp)const
{
  return operator>=((const char*)cmp);
}

int String::operator>=(const char* cmp)const
{
  int i;
  if(_nocase)
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(tolower(_str[i])<tolower(cmp[i]))return 0;
    }
  }else
  {
    for(i=0;i<_length && cmp[i];i++)
    {
      if(_str[i]<cmp[i])return 0;
    }
  }
  if(i>=_length && cmp[i])return 0;
  return 1;
}


String String::operator+(const String& src)const
{
  String tmp(*this);
  tmp+=src;
  return tmp;
}

String String::operator+(const char* src)const
{
  String tmp(*this);
  tmp+=src;
  return tmp;
}

String String::operator+(const char src)const
{
  String tmp(*this);
  tmp+=src;
  return tmp;
}

String String::operator+(int src)const
{
  String tmp(*this);
  tmp+=src;
  return tmp;
}

void String::operator+=(const String& src)
{
  Resize(_length+src.Length());
  xmemcpy(_str+_length,(const char*)src,src.Length()+1);
  _length+=src.Length();
}

void String::operator+=(const char* src)
{
  int srclength=(int)strlen(src);
  Resize(_length+srclength);
  xmemcpy(_str+_length,src,srclength+1);
  _length+=srclength;
}

void String::operator+=(const char src)
{
  Resize(_length+1);
  _str[_length]=src;
  _length++;
  _str[_length]=0;
}

void String::operator+=(int src)
{
  char buf[64];
  sprintf(buf,"%d",src);
  (*this)+=buf;
}

String& String::operator<<(const String& src)
{
  (*this)+=src;
  return *this;
}
String& String::operator<<(const char* src)
{
  (*this)+=src;
  return *this;
}
String& String::operator<<(char src)
{
  (*this)+=src;
  return *this;
}
String& String::operator<<(int src)
{
  (*this)+=src;
  return *this;
}

String String::operator*(int num)
{
  String tmp;
  tmp.SetSize(_length*num);
  int i;
  for(i=0;i<num;i++)tmp+=*this;
  return tmp;
}

void String::operator*=(int num)
{
  int newlength=_length*num;
  Resize(newlength);
  int i,pos=_length;
  for(i=1;i<num;i++)
  {
    memcpy(_str+pos,_str,_length);
    pos+=_length;
  }
  _length=newlength;
  _str[_length]=0;
}

/*String String::Substr(int index,int count)
{
  String tmp;
  tmp.Set(_str,index,count);
  return tmp;
}*/

String String::Substr(int index,int count)const
{
  String tmp;
  tmp.Set(_str,index,count);
  return tmp;
}

void String::Set(const char* src,int index,int count)
{
  int srclength=(int)strlen(src);
  if(index>=srclength)
  {
    _length=0;
    if(_str)_str[0]=0;
    return;
  }
  if(index<0)index+=srclength;
  if(index<0)
  {
    _length=0;
    if(_str)_str[0]=0;
    return;
  }
  if(count<0)count=srclength;
  if(count>srclength-index)count=srclength-index;
  SetSize(count);
  memcpy(_str,src+index,count);
  _length=count;
  _str[_length]=0;
}

void String::Concat(const char* src,int index,int count)
{
  int srclength=(int)strlen(src);
  if(index>=srclength)
  {
    _length=0;
    if(_str)_str[0]=0;
    return;
  }
  if(index<0)index+=srclength;
  if(index<0)
  {
    _length=0;
    if(_str)_str[0]=0;
    return;
  }
  if(count<0)count=srclength;
  if(count>srclength-index)count=srclength-index;
  Resize(_length+count);
  memcpy(_str+_length,src+index,count);
  _length+=count;
  _str[_length]=0;
}


int String::Index(const String& substr,int pos)const
{
  return _Index((const char*)substr,substr.Length(),pos);
}

int String::Index(const char* substr,int pos)const
{
  return _Index(substr,(int)strlen(substr),pos);
}

int String::_Index(const char* substr,int substrlength,int pos)const
{
//  int substrlength=strlen(substr);
  if(pos<0)pos+=_length;
  if(pos<0 || pos>_length)return -1;
  int i=pos,j;
  if(_nocase)
  {
    while(i+substrlength<=_length)
    {
      if(tolower(_str[i])==tolower(substr[0]))
      {
        if(substrlength==1)return i;
        j=1;
        while(tolower(_str[i+j])==tolower(substr[j]))
        {
          j++;
          if(j==substrlength)return i;
        }
      }
      i++;
    }
  }else
  {
    while(i+substrlength<=_length)
    {
      if(_str[i]==substr[0])
      {
        if(substrlength==1)return i;
        j=1;
        while(_str[i+j]==substr[j])
        {
          j++;
          if(j==substrlength)return i;
        }
      }
      i++;
    }
  }
  return -1;
}

int String::_RIndex(const char* substr,int substrlength,int pos)const
{
//  int substrlength=strlen(substr);
  if(pos<0)pos+=_length;
  if(pos<0 || pos>_length)return -1;
  int i=pos,j;
  if(_nocase)
  {
    while(i>=0)
    {
      if(tolower(_str[i])==tolower(substr[0]))
      {
        if(substrlength==1)return i;
        j=1;
        while(tolower(_str[i+j])==tolower(substr[j]))
        {
          j++;
          if(j==substrlength)return i;
        }
      }
      i--;
    }
  }else
  {
    while(i>=0)
    {
      if(_str[i]==substr[0])
      {
        if(substrlength==1)return i;
        j=1;
        while(_str[i+j]==substr[j])
        {
          j++;
          if(j==substrlength)return i;
        }
      }
      i--;
    }
  }
  return -1;
}

int String::RIndex(const String& substr,int pos)const
{
  return _RIndex(substr,substr.Length(),pos);
}

int String::RIndex(const char* substr,int pos)const
{
  return _RIndex(substr,(int)strlen(substr),pos);
}


void String::Insert(int index,const String& ins)
{
  int newlength=_length+ins.Length();
  if(index<0)index+=_length;
  if(index>_length)
  {
    (*this)+=ins;
    return;
  }
  Resize(newlength);
  memmove(_str+index+ins.Length(),_str+index,_length-index+1);
  xmemcpy(_str+index,(const char*)ins,ins.Length());
  _length=newlength;
}

void String::Insert(int index,const char* ins)
{
  int inslength=(int)strlen(ins);
  int newlength=_length+inslength;
  if(index<0)index+=_length;
  if(index>_length)
  {
    (*this)+=ins;
    return;
  }
  Resize(newlength);
  memmove(_str+index+inslength,_str+index,_length-index+1);
  xmemcpy(_str+index,ins,inslength);
  _length=newlength;
}

void String::Delete(int index,int count)
{
  if(index<0)index+=_length;
  if(index<0)return;
  if(count<0 || index+count>_length)
  {
    _length=index;
    _str[_length]=0;
    return;
  }
  memmove(_str+index,_str+index+count,_length-index-count+1);
  _length-=count;
}

int String::Replace(const String& src,const String& dst)
{
  String tmp;
  int i=0,j=0,cnt=0;
  while((i=Index(src,i))!=-1)
  {
    tmp.Concat(_str,j,i-j);
    tmp<<dst;
    i+=src.Length();
    j=i;
    cnt++;
  }
  if(j<_length)tmp.Concat(_str,j,_length-j);
//  if(
  (*this)=tmp;
  return cnt;
}

int String::Replace(const char* src,const char* dst)
{
  String tmp;
  int srclength=(int)strlen(src);
  int i=0,j=0,cnt=0;
  while((i=Index(src,i))!=-1)
  {
    tmp.Concat(_str,j,i-j);
    tmp<<dst;
    i+=srclength;
    j=i;
    cnt++;
  }
  if(j<_length)tmp.Concat(_str,j,_length-j);
  (*this)=tmp;
  return cnt;
}

int String::Replace(char from,char to)
{
  int cnt=0;
  for(int i=0;i<_length;i++)
  {
    if(_str[i]==from)
    {
      _str[i]=to;
      cnt++;
    }
  }
  return cnt;
}

void String::Fill(char src,int num)
{
  SetSize(num);
  memset(_str,src,num);
  _length=num;
  _str[_length]=0;
}

int String::Sprintf(const char *fmt,...)
{
  SetSize((int)strlen(fmt)*4);
  va_list arglist;
  int res;
  do{
    va_start(arglist,fmt);
#ifdef _WIN32
    res=_vsnprintf( _str, _size,fmt,arglist);
#else
    res=vsnprintf( _str, _size,fmt,arglist);
#endif
    va_end(arglist);
    if(res<0)
    {
      SetSize(_size-res);
    }
  }while(res<0);
  _length=res;
  _str[_length]=0;
  return res;
}

int String::AppendSprintf(const char *fmt,...)
{
  Resize(_length+(int)strlen(fmt)*4);
  va_list arglist;
  int res;
  do{
    va_start(arglist,fmt);
#ifdef _WIN32
    res=_vsnprintf(_str+_length,_size-_length,fmt,arglist);
#else
    res=vsnprintf(_str+_length,_size-_length,fmt,arglist);
#endif
    va_end(arglist);
    if(res<0)
    {
      Resize(_size-res);
    }
  }while(res<0);
  _length+=res;
  return res;
}


void String::Clean()
{
  delete [] _str;
  _str=0;
  _size=0;
  _length=0;
}

void String::SetLength(int newlength)
{
  Resize(newlength);
  _length=newlength;
  _str[newlength]=0;
}

void String::RTrim()
{
  while(_length>0 && isspace(_str[_length-1]))_length--;
  _str[_length]=0;
}

void String::LTrim()
{
  int i=0;
  while(i<_length && isspace(_str[i]))i++;
  if(i==_length)
  {
    _length=0;
    _str[0]=0;
  }else
  {
    memmove(_str,_str+i,_length-i);
    _length-=i;
    _str[_length]=0;
  }
}

void String::Align(int where,int size)
{
  if(size<_length)return;
  Resize(size);
  switch(where)
  {
    case apLeft:
    {
      memset(_str+_length,' ',size-_length);
      break;
    }
    case apCenter:
    {
      int left=(size-_length)/2;
      int right=size-_length-left;
      memmove(_str+left,_str,_length);
      memset(_str,' ',left);
      memset(_str+left+_length,' ',right);
      break;
    }
    case apRight:
    {
      int fill=size-_length;
      memmove(_str+fill,_str,_length);
      memset(_str,' ',fill);
      break;
    }
    case apWords:
    {
      int i=0;
      Trim();
      //while(isspace(_str[i]) && i<_length)i++;
      if(i==_length)return;
      int words=0;
      int spaces=0;
      while(i<_length)
      {
        while(!isspace(_str[i]) && i<_length)i++;
        while(isspace(_str[i]) && i<_length)
        {
          i++;
          spaces++;
        }
        words++;
      }
      if(words==1)
      {
        return;
      }
      int fill=size-_length+spaces;
      int perword=fill/(words-1);
      int rest=fill-perword*(words-1);
      int moreevery=rest?(words-1)/rest:-1;
      String old=*this;
      _length=0;
      i=0;
      int j,k=0,pad;
      while(i<old._length)
      {
        j=i;
        while(!isspace(old._str[i]) && i<old._length)i++;
        if(i==old._length)
        {
          memmove(_str+size-(i-j),old._str+j,i-j);
          break;
        }
        memmove(_str+_length,old._str+j,i-j);
        _length+=i-j;
        k++;
        pad=perword;
        if(k==moreevery)
        {
          pad++;
          k=0;
        }
        memset(_str+_length,' ',pad);
        _length+=pad;
        while(isspace(old._str[i]) && i<old._length)i++;
      }
      break;
    }
  }
  _length=size;
  _str[_length]=0;
}


int String::LoadFromFile(const char* filename)
{
  FILE *f=fopen(filename,"rb");
  if(!f)return 0;
  fseek(f,0,SEEK_END);
  int size=(int)ftell(f);
  fseek(f,0,SEEK_SET);
  SetSize(size);
  if(fread(_str,1,size,f)!=size_t(size))
  {
    fclose(f);
    return 0;
  };
  fclose(f);
  _str[size]=0;
  _length=size;
  return 1;
}

String& String::ToLower()
{
  for(int i=0;i<_length;i++)_str[i]=tolower(_str[i]);
  return *this;
}

String& String::ToUpper()
{
  for(int i=0;i<_length;i++)_str[i]=toupper(_str[i]);
  return *this;
}

int String::ToInt()const
{
  int rv=-1;
  sscanf(_str,"%d",&rv);
  return rv;
}

}//buffers
}//core
}//smsc
