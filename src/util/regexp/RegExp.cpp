/*
  Copyright (c) Konstantin Stupnik (aka Xecutor) 2001 <skv@novosoft.ru>
  You can use, modify, distribute this code or any other part
  only with permissions of author or Novosoft Inc!

  Regular expressions support library.
  Syntax and semantics of regexps very close to
  syntax and semantics of perl regexps.

*/

#if defined(RE_SPINOZA_MODE)
  #ifdef __BORLANDC__
    #include "CRegExp/RegExp.hpp"
  #else
    #include "RegExp.hpp"
  #endif
#else
  #include "RegExp.hpp"
#endif

#ifndef RE_EXTERNAL_CTYPE
#include <ctype.h>
#endif
#ifndef RE_NO_STRING_H
#include <string.h>
#else
 #ifndef RE_SPINOZA_MODE
   extern "C" void* memcpy(void*,void*,unsigned int);
   extern "C" void* memset(void*,int,unsigned int);
   extern "C" unsigned int strlen(const char*);
   extern "C" int strcmp(const char*,const char*);
 #endif
#endif
//#include <stdlib.h>

#ifdef RE_SPINOZA_MODE
 #define dpf(x)
#else
 #ifdef _DEBUG
   #include <stdio.h>
   #ifdef dpf
     #undef dpf
   #endif
   #define dpf(x) printf x

static char *ops[]={
  "opNone",
  "opLineStart",
  "opLineEnd",
  "opDataStart",
  "opDataEnd",
  "opWordBound",
  "opNotWordBound",
  "opType",
  "opNotType",
  "opCharAny",
  "opCharAnyAll",
  "opSymbol",
  "opNotSymbol",
  "opSymbolIgnoreCase",
  "opNotSymbolIgnoreCase",
  "opSymbolClass",
  "opOpenBracket",
  "opClosingBracket",
  "opAlternative",
  "opBackRef",
#ifdef NAMEDBRACKETS
  "opNamedBracket",
  "opNamedBackRef",
#endif
  "opRangesBegin",
  "opRange",
  "opMinRange",
  "opSymbolRange",
  "opSymbolMinRange",
  "opNotSymbolRange",
  "opNotSymbolMinRange",
  "opAnyRange",
  "opAnyMinRange",
  "opTypeRange",
  "opTypeMinRange",
  "opNotTypeRange",
  "opNotTypeMinRange",
  "opClassRange",
  "opClassMinRange",
  "opBracketRange",
  "opBracketMinRange",
  "opBackRefRange",
  "opBackRefMinRange",
#ifdef NAMEDBRACKETS
  "opNamedRefRange",
  "opNamedRefMinRange",
#endif
  "opRangesEnd",
  "opAssertionsBegin",
  "opLookAhead",
  "opNotLookAhead",
  "opLookBehind",
  "opNotLookBehind",
  "opAsserionsEnd",
  "opNoReturn",
#ifdef RELIB
  "opLibCall",
#endif
  "opRegExpEnd",
};

 #else
   #define dpf(x)
 #endif
#endif // RE_SPINOZA_MODE

namespace smsc{
namespace util{
namespace regexp{


enum REOp{
  opLineStart=0x1,        // ^
  opLineEnd,              // $
  opDataStart,            // \A and ^ in single line mode
  opDataEnd,              // \Z and $ in signle line mode

  opWordBound,            // \b
  opNotWordBound,         // \B

  opType,                 // \d\s\w\l\u\e
  opNotType,              // \D\S\W\L\U\E

  opCharAny,              // .
  opCharAnyAll,           // . in single line mode

  opSymbol,               // single char
  opNotSymbol,            // [^c] negative charclass with one char
  opSymbolIgnoreCase,     // symbol with IGNORE_CASE turned on
  opNotSymbolIgnoreCase,  // [^c] with ignore case set.

  opSymbolClass,          // [chars]

  opOpenBracket,          // (

  opClosingBracket,       // )

  opAlternative,          // |

  opBackRef,              // \1

#ifdef NAMEDBRACKETS
  opNamedBracket,         // (?{name}
  opNamedBackRef,         // \p{name}
#endif


  opRangesBegin,          // for op type check

  opRange,                // generic range
  opMinRange,             // generic minimizing range

  opSymbolRange,          // quantifier applied to single char
  opSymbolMinRange,       // minimizing quantifier

  opNotSymbolRange,       // [^x]
  opNotSymbolMinRange,

  opAnyRange,             // .
  opAnyMinRange,

  opTypeRange,            // \w, \d, \s
  opTypeMinRange,

  opNotTypeRange,         // \W, \D, \S
  opNotTypeMinRange,

  opClassRange,           // for char classes
  opClassMinRange,

  opBracketRange,         // for brackets
  opBracketMinRange,

  opBackRefRange,         // for backrefs
  opBackRefMinRange,

#ifdef NAMEDBRACKETS
  opNamedRefRange,
  opNamedRefMinRange,
#endif

  opRangesEnd,            // end of ranges

  opAssertionsBegin,

  opLookAhead,
  opNotLookAhead,

  opLookBehind,
  opNotLookBehind,

  opAsserionsEnd,

  opNoReturn,

#ifdef RELIB
  opLibCall,
#endif
  opRegExpEnd,
};

struct REOpCode{
  int op;
  REOpCode *next,*prev;
#ifdef _DEBUG
  int    srcpos;
#endif
  #ifdef RE_NO_NEWARRAY
  static void OnCreate(void *ptr);
  static void OnDelete(void *ptr);
  #else
  REOpCode()
  {
    memset(this,0,sizeof(*this));
  }
  ~REOpCode();
  #endif
  union{
    struct{
      union{
        struct{
          REOpCode* nextalt;
          int index;
          REOpCode* pairindex;
        }bracket;
        int op;
        rechar symbol;
        prechar symbolclass;
        REOpCode* nextalt;
        int refindex;
#ifdef NAMEDBRACKETS
        prechar refname;
#endif
        int type;
      };
      int min,max;
    }range;
    struct{
      REOpCode* nextalt;
      int index;
      REOpCode* pairindex;
    }bracket;
#ifdef NAMEDBRACKETS
    struct{
      REOpCode* nextalt;
      prechar name;
      REOpCode* pairindex;
    }nbracket;
#endif
    struct{
      REOpCode* nextalt;
      int length;
      REOpCode* pairindex;
    }assert;
    struct{
      REOpCode* nextalt;
      REOpCode* endindex;
    }alternative;
    rechar symbol;
    prechar symbolclass;
    int refindex;
#ifdef NAMEDBRACKETS
    prechar refname;
#endif
#ifdef RELIB
    prechar rename;
#endif

    int type;
  };
};

#ifdef RE_NO_NEWARRAY
void StateStackItem::OnCreate(void *ptr)
{
  memset(ptr,0,sizeof(StateStackItem));
}

void REOpCode::OnCreate(void *ptr)
{
  memset(ptr,0,sizeof(REOpCode));
}

void REOpCode::OnDelete(void *ptr)
{
  REOpCode &o=*static_cast<REOpCode*>(ptr);
  switch(o.op)
  {
    case opSymbolClass:
      if(o.symbolclass)
        free(o.symbolclass);
      break;
    case opClassRange:
    case opClassMinRange:
      if(o.range.symbolclass)
        free(o.range.symbolclass);
      break;
    #ifdef NAMEDBRACKETS
    case opNamedBracket:
      if(o.nbracket.name)
        free(o.nbracket.name);
      break;
    case opNamedBackRef:
      if(o.refname)
        free(o.refname);
      break;
    #endif
    #ifdef RELIB
    case opLibCall:
      if(o.rename)
        free(o.rename);
      break;
    #endif
  }
}

void *RegExp::CreateArray(const unsigned int size, const unsigned int total,
                          ON_CREATE_FUNC Create)
{
  if(total && size)
  {
    /* record[0] - sizeof
       record[1] - total
       record[2] - array
    */
    unsigned char *record=static_cast<unsigned char*>
      (malloc(sizeof(unsigned int)*2+size*total));
    if(record)
    {
      unsigned char *array=record+2*sizeof(unsigned int);
      *reinterpret_cast<int*>(record)=size;
      *reinterpret_cast<int*>(record+sizeof(unsigned int))=total;
      if(Create!=NULL)
        for(unsigned int f=0;f<total;++f)
          Create(array+size*f);
      return reinterpret_cast<void*>(array);
    }
  }
  return NULL;
}

void RegExp::DeleteArray(void **array, ON_DELETE_FUNC Delete)
{
  if(array && *array)
  {
    unsigned char *record=reinterpret_cast<unsigned char*>(*array)-
      2*sizeof(unsigned int);
    if(Delete!=NULL)
    {
      unsigned char *m=static_cast<unsigned char*>(*array);
      unsigned int size=*reinterpret_cast<int*>(record),
        total=*reinterpret_cast<int*>(record+sizeof(unsigned int));
      for(unsigned int f=0;f<total;++f)
        Delete(m+size*f);
    }
    free(record);
    *array=NULL;
  }
}
#else  // RE_NO_NEWARRAY
REOpCode::~REOpCode()
{
  switch(op)
  {
    case opSymbolClass:delete [] symbolclass;break;
    case opClassRange:
    case opClassMinRange:delete [] range.symbolclass;break;
#ifdef NAMEDBRACKETS
    case opNamedBracket:delete [] nbracket.name;break;
    case opNamedBackRef:delete [] refname;break;
#endif
#ifdef RELIB
    case opLibCall:delete [] rename;break;
#endif
  }
}
#endif // RE_NO_NEWARRAY

#ifdef RE_STATIC_LOCALE
#ifdef RE_EXTERNAL_CTYPE
prechar RegExp::lc;
prechar RegExp::uc;
prechar RegExp::chartypes;
#else
rechar RegExp::lc[256];
rechar RegExp::uc[256];
rechar RegExp::chartypes[256];
#endif
rechar RegExp::charbits[256];
#endif


void RegExp::Init(const prechar expr,int options)
{
  //memset(this,0,sizeof(*this));
  code=NULL;
#ifndef RE_STATIC_LOCALE
  #ifndef RE_EXTERNAL_CTYPE
    InitLocale();
  #endif
#endif
#ifdef NAMEDBRACKETS
  havenamedbrackets=0;
#endif
  stack=&initstack[0];
  st=&stack[0];
  initstackpage.stack=stack;
  firstpage=lastpage=&initstackpage;
  firstpage->next=NULL;
  firstpage->prev=NULL;

  start=NULL;
  end=NULL;
  trimend=NULL;

  Compile((char*)expr,options);
}

RegExp::RegExp()
{
  //memset(this,0,sizeof(*this));
  code=NULL;
#ifndef RE_STATIC_LOCALE
  #ifndef RE_EXTERNAL_CTYPE
    InitLocale();
  #endif
#endif
#ifdef NAMEDBRACKETS
  havenamedbrackets=0;
#endif
  stack=&initstack[0];
  st=&stack[0];
  initstackpage.stack=stack;
  firstpage=lastpage=&initstackpage;
  firstpage->next=NULL;
  firstpage->prev=NULL;

  start=NULL;
  end=NULL;
  trimend=NULL;

  errorcode=errNotCompiled;
  #ifdef _DEBUG
  resrc=NULL;
  #endif
}

RegExp::RegExp(const char* expr,int options)
{
  #ifdef _DEBUG
  resrc=NULL;
  #endif
  Init((const prechar)expr,options);
}

RegExp::~RegExp()
{
  #ifdef _DEBUG
    #ifdef RE_NO_NEWARRAY
      if(resrc)
        free(resrc);
    #else
      delete [] resrc;
    #endif // RE_NO_NEWARRAY
  #endif
  if(code)
  {
    #ifdef RE_NO_NEWARRAY
    DeleteArray(reinterpret_cast<void**>(&code),REOpCode::OnDelete);
    #else
    delete [] code;
    code=NULL;
    #endif
  }
  if(firstpage!=lastpage)
  {
    CleanStack();
  }
}

#ifndef RE_EXTERNAL_CTYPE
void RegExp::InitLocale()
{
  int i;
  for(i=0;i<256;i++)
  {
    lc[i]=tolower(i);
    uc[i]=toupper(i);
  }
  for(i=0;i<256;i++)
  {
    char res=0;
    if(isalnum(i) || i=='_')res|=TYPE_WORDCHAR;
    if(isalpha(i))res|=TYPE_ALPHACHAR;
    if(isdigit(i))res|=TYPE_DIGITCHAR;
    if(isspace(i))res|=TYPE_SPACECHAR;
    if(lc[i]==i && uc[i]!=i)res|=TYPE_LOWCASE;
    if(uc[i]==i && lc[i]!=i)res|=TYPE_UPCASE;
    chartypes[i]=res;
  }
  int j=0,k=1;
  memset(charbits,0,sizeof(charbits));
  for(i=0;i<256;i++)
  {
    if(chartypes[i]&TYPE_DIGITCHAR){charbits[j]|=k;}
    if(chartypes[i]&TYPE_SPACECHAR){charbits[32+j]|=k;}
    if(chartypes[i]&TYPE_WORDCHAR){charbits[64+j]|=k;}
    if(chartypes[i]&TYPE_LOWCASE){charbits[96+j]|=k;}
    if(chartypes[i]&TYPE_UPCASE){charbits[128+j]|=k;}
    if(chartypes[i]&TYPE_ALPHACHAR){charbits[160+j]|=k;}
    k<<=1;
    if(k==256){k=1;j++;}
  }
}
#endif

int RegExp::CalcLength(const prechar src,int srclength)
{
  int length=3;//global brackets
  int brackets[MAXDEPTH];
  int count=0;
  int i,save;
  bracketscount=1;
  int inquote=0;

  for(i=0;i<srclength;i++,length++)
  {
    if(inquote && src[i]!='\\' && src[i+1]!='E')
    {
      continue;
    }
    switch(src[i])
    {
      case '\\':
      {
        i++;
        if(src[i]=='Q')inquote=1;
        if(src[i]=='E')inquote=0;
        if(src[i]=='x')
        {
          i++;
          int c=lc[src[i]];
          if((c>='0' && c<='9')||(c>='a' && c<='f'))
          {
            int c=lc[src[i+1]];
            if((c>='0' && c<='9')||(c>='a' && c<='f'))i++;

          }else return SetError(errSyntax,i);
        }
#ifdef NAMEDBRACKETS
        if(src[i]=='p')
        {
          i++;
          if(src[i]!='{')
            return SetError(errSyntax,i);
          i++;
          int save=i;
          while(i<srclength && (chartypes[src[i]]&(TYPE_WORDCHAR|TYPE_SPACECHAR)) && src[i]!='}')
            i++;
          if(i>=srclength)
            return SetError(errBrackets,save);
          if(src[i]!='}' && !(chartypes[src[i]]&(TYPE_WORDCHAR|TYPE_SPACECHAR)))
            return SetError(errSyntax,i);
        }
#endif
        break;
      }
      case '(':
      {
        brackets[count]=i;
        count++;
        if(count==MAXDEPTH)return SetError(errMaxDepth,i);
        if(src[i+1]=='?')
        {
          i+=2;
#ifdef NAMEDBRACKETS
          if(src[i]=='{')
          {
            save=i;
            i++;
            while(i<srclength && (chartypes[src[i]]&(TYPE_WORDCHAR|TYPE_SPACECHAR)) && src[i]!='}')
              i++;
            if(i>=srclength)
              return SetError(errBrackets,save);
            if(src[i]!='}' && !(chartypes[src[i]]&(TYPE_WORDCHAR|TYPE_SPACECHAR)))
              return SetError(errSyntax,i);
          }
#endif
        }else
        {
          bracketscount++;
        }
        break;
      }
      case ')':
      {
        count--;
        if(count<0)return SetError(errBrackets,i);
        break;
      }
      case '{':
      case '*':
      case '+':
      case '?':
      {
        length--;
        if(src[i]=='{')
        {
          save=i;
          while(i<srclength && src[i]!='}')i++;
          if(i>=srclength)return SetError(errBrackets,save);
        }
        if(src[i+1]=='?')i++;
        break;
      }
      case '[':
      {
        save=i;
        while(i<srclength && src[i]!=']')i++;
        if(i>=srclength)return SetError(errBrackets,save);
        break;
      }
#ifdef RELIB
      case '%':
      {
        i++;
        save=i;
        while(i<srclength && src[i]!='%')i++;
        if(i>=srclength)return SetError(errBrackets,save-1);
        if(save==i)return SetError(errSyntax,save);
      }break;
#endif
    }
  }
  if(count)
  {
    errorpos=brackets[0];
    errorcode=errBrackets;
    return 0;
  }
  return length;
}

int RegExp::Compile(const char* src,int options)
{
  int srcstart=0,srclength/*=0*/,relength;
  havefirst=0;
  #ifdef RE_NO_NEWARRAY
  DeleteArray(reinterpret_cast<void**>(&code),REOpCode::OnDelete);
  #else
  if(code)delete [] code;
  code=NULL;
  #endif
  if(options&OP_PERLSTYLE)
  {
    if(src[0]!='/')return SetError(errSyntax,0);
    srcstart=1;
    srclength=1;
    while(src[srclength] && src[srclength]!='/')
    {
      if(src[srclength]=='\\' && src[srclength+1]!=0)
      {
        srclength++;
      }
      srclength++;
    }
    if(!src[srclength])
    {
      return SetError(errSyntax,srclength-1);
    }
    int i=srclength+1;
    srclength--;
    while(src[i])
    {
      switch(src[i])
      {
        case 'i':options|=OP_IGNORECASE;break;
        case 's':options|=OP_SINGLELINE;break;
        case 'm':options|=OP_MULTILINE;break;
        case 'x':options|=OP_XTENDEDSYNTAX;break;
        case 'o':options|=OP_OPTIMIZE;break;
        default:return SetError(errOptions,i);
      }
      i++;
    }
  }else
  {
    srclength=strlen(src);
  }
  ignorecase=options&OP_IGNORECASE?1:0;
  relength=CalcLength((const prechar)src+srcstart,srclength);
  if(relength==0)
  {
    return 0;
  }
  #ifdef RE_NO_NEWARRAY
  code=static_cast<REOpCode*>
    (CreateArray(sizeof(REOpCode), relength, REOpCode::OnCreate));
  #else
  code=new REOpCode[relength];
  memset(code,0,sizeof(REOpCode)*relength);
  #endif
  for(int i=0;i<relength;i++)
  {
    code[i].next=i<relength-1?code+i+1:0;
    code[i].prev=i>0?code+i-1:0;
  }
  int result=InnerCompile((const prechar)src+srcstart,srclength,options);
  if(!result)
  {
    #ifdef RE_NO_NEWARRAY
    DeleteArray(reinterpret_cast<void**>(&code),REOpCode::OnDelete);
    #else
    delete [] code;
    code=NULL;
    #endif
  }else
  {
    errorcode=errNone;
    if(options&OP_OPTIMIZE)Optimize();
  }
  return result;
}

int RegExp::GetNum(const prechar src,int& i)
{
  int res=0;//atoi((const char*)src+i);
  while(chartypes[src[i]]&TYPE_DIGITCHAR)
  {
    res*=10;
    res+=src[i]-'0';
    i++;
  }
  return res;
}

void inline RegExp::SetBit(prechar bitset,int charindex)
{
  bitset[charindex>>3]|=1<<(charindex&7);
}
int inline RegExp::GetBit(prechar bitset,int charindex)
{
  return bitset[charindex>>3]&(1<<(charindex&7));
}

int RegExp::InnerCompile(const prechar src,int srclength,int options)
{
  int i,j;
  PREOpCode brackets[MAXDEPTH];
  // current brackets depth
  // one place reserved for surrounding 'main' brackets
  int brdepth=1;

  // compiling interior of lookbehind
  // used to apply restrictions of lookbehind
  int lookbehind=0;

  // counter of normal brackets
  int brcount=0;

  // counter of closed brackets
  // used to check correctness of backreferences
  bool closedbrackets[MAXDEPTH];


  // quoting is active
  int inquote=0;

  maxbackref=0;

  rechar tmpclass[32];
  int *itmpclass=(int*)tmpclass;

  code->op=opOpenBracket;
  code->bracket.index=0;

#ifdef NAMEDBRACKETS
  MatchHash h;
  SMatch m;
#endif

  int pos=1;

  register PREOpCode op;//=code;

  brackets[0]=code;
#ifdef _DEBUG
  #ifdef RE_NO_NEWARRAY
    resrc=static_cast<rechar*>(malloc(sizeof(rechar)*(srclength+4)));
  #else
  resrc=new rechar[srclength+4];
  #endif // RE_NO_NEWARRAY
  resrc[0]='(';
  resrc[1]=0;
  memcpy(resrc+1,src,srclength);
  resrc[srclength+1]=')';
  resrc[srclength+2]=27;
  resrc[srclength+3]=0;
#endif

  havelookahead=0;
  for(i=0;i<srclength;i++)
  {
    op=code+pos;
    pos++;
#ifdef _DEBUG
    op->srcpos=i+1;
#endif
    if(inquote && src[i]!='\\')
    {
      op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
      op->symbol=ignorecase?lc[src[i]]:src[i];
      if(ignorecase && uc[op->symbol]==op->symbol)op->op=opSymbol;
      continue;
    }
    switch(src[i])
    {
      case '\\':
      {
        i++;
        if(inquote && src[i]!='E')
        {
          op->op=opSymbol;
          op->symbol='\\';
          op=code+pos;
          pos++;
          op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
          op->symbol=ignorecase?lc[src[i]]:src[i];
          if(ignorecase && uc[op->symbol]==op->symbol)op->op=opSymbol;
          continue;
        }
        op->op=opType;
        switch(src[i])
        {
          case 'Q':inquote=1;pos--;continue;
          case 'E':inquote=0;pos--;continue;

          case 'b':op->op=opWordBound;continue;
          case 'B':op->op=opNotWordBound;continue;
          case 'D':op->op=opNotType;
          case 'd':op->type=TYPE_DIGITCHAR;continue;
          case 'S':op->op=opNotType;
          case 's':op->type=TYPE_SPACECHAR;continue;
          case 'W':op->op=opNotType;
          case 'w':op->type=TYPE_WORDCHAR;continue;
          case 'U':op->op=opNotType;
          case 'u':op->type=TYPE_UPCASE;continue;
          case 'L':op->op=opNotType;
          case 'l':op->type=TYPE_LOWCASE;continue;
          case 'I':op->op=opNotType;
          case 'i':op->type=TYPE_ALPHACHAR;continue;
          case 'A':op->op=opDataStart;continue;
          case 'Z':op->op=opDataEnd;continue;
          case 'n':op->op=opSymbol;op->symbol='\n';continue;
          case 'r':op->op=opSymbol;op->symbol='\r';continue;
          case 't':op->op=opSymbol;op->symbol='\t';continue;
          case 'f':op->op=opSymbol;op->symbol='\f';continue;
          case 'e':op->op=opSymbol;op->symbol=27;continue;
          case 'O':op->op=opNoReturn;continue;
#ifdef NAMEDBRACKETS
          case 'p':
          {
            op->op=opNamedBackRef;
            i++;
            if(src[i]!='{')return SetError(errSyntax,i);
            int len=0;i++;
            while(src[i+len]!='}')len++;
            if(len>0)
            {
              #ifdef RE_NO_NEWARRAY
              op->refname=static_cast<rechar*>(malloc(sizeof(rechar)*(len+1)));
              #else
              op->refname=new rechar[len+1];
              #endif
              memcpy(op->refname,src+i,len);
              op->refname[len]=0;
              if(!h.Exists((char*)op->refname))
              {
                return SetError(errReferenceToUndefinedNamedBracket,i);
              }
              i+=len;
            }else
            {
              return SetError(errSyntax,i);
            }
          }continue;
#endif
          case 'x':
          {
            int c=0;
            i++;
            if(i>=srclength)return SetError(errSyntax,i-1);
            c=lc[src[i]];
            if((c>='0' && c<='9') ||
               (c>='a' && c<='f'))
            {
              c-='0';
              if(c>9)c-='a'-'0'-10;
              op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
              op->symbol=c;
              if(i+1<srclength)
              {
                c=lc[src[i+1]];
                if((c>='0' && c<='9') ||
                   (c>='a' && c<='f'))
                {
                  i++;
                  c-='0';
                  if(c>9)c-='a'-'0'-10;
                  op->symbol<<=4;
                  op->symbol|=c;
                }
                if(ignorecase)
                {
                  op->symbol=lc[op->symbol];
                  if(uc[op->symbol]==lc[op->symbol])
                  {
                    op->op=opSymbol;
                  }
                }

              }
            }else return SetError(errSyntax,i);
            continue;
          }
          default:
          {
            if(chartypes[src[i]]&TYPE_DIGITCHAR)
            {
              int save=i;
              op->op=opBackRef;
              op->refindex=GetNum(src,i);i--;
              if(op->refindex<=0 || op->refindex>brcount || !closedbrackets[op->refindex])
              {
                return SetError(errInvalidBackRef,save-1);
              }
              if(op->refindex>maxbackref)maxbackref=op->refindex;
            }else
            {
              if(options&OP_STRICT && chartypes[src[i]]&TYPE_ALPHACHAR)
              {
                return SetError(errInvalidEscape,i-1);
              }
              op->op=ignorecase?opSymbolIgnoreCase:opSymbol;
              op->symbol=ignorecase?lc[src[i]]:src[i];
              if(uc[op->symbol]==lc[op->symbol])
              {
                op->op=opSymbol;
              }
            }
          }
        }
      }continue;
      case '.':
      {
        if(options&OP_SINGLELINE)
        {
          op->op=opCharAnyAll;
        }else
        {
          op->op=opCharAny;
        }
        continue;
      }
      case '^':
      {
        if(options&OP_MULTILINE)
        {
          op->op=opLineStart;
        }else
        {
          op->op=opDataStart;
        }
        continue;
      }
      case '$':
      {
        if(options&OP_MULTILINE)
        {
          op->op=opLineEnd;
        }else
        {
          op->op=opDataEnd;
        }
        continue;
      }
      case '|':
      {
        if(lookbehind)return SetError(errSyntax,i);
        if(brackets[brdepth-1]->op==opAlternative)
        {
          brackets[brdepth-1]->alternative.nextalt=op;
        }else
        {
          if(brackets[brdepth-1]->op==opOpenBracket)
          {
            brackets[brdepth-1]->bracket.nextalt=op;
          }else
          {
            brackets[brdepth-1]->assert.nextalt=op;
          }
        }
        if(brdepth==MAXDEPTH)return SetError(errMaxDepth,i);
        brackets[brdepth++]=op;
        op->op=opAlternative;
        continue;
      }
      case '(':
      {
        if(lookbehind)return SetError(errSyntax,i);
        op->op=opOpenBracket;
        if(src[i+1]=='?')
        {
          i+=2;
          switch(src[i])
          {
            case ':':op->bracket.index=-1;break;
            case '=':op->op=opLookAhead;havelookahead=1;break;
            case '!':op->op=opNotLookAhead;havelookahead=1;break;
            case '<':
            {
              i++;
              if(src[i]=='=')
              {
                op->op=opLookBehind;
              }else if(src[i]=='!')
              {
                op->op=opNotLookBehind;
              }else return SetError(errSyntax,i);
              lookbehind=1;
            }break;
#ifdef NAMEDBRACKETS
            case '{':
            {
              op->op=opNamedBracket;
              havenamedbrackets=1;
              int len=0;
              i++;
              while(src[i+len]!='}')len++;
              if(len>0)
              {
                #ifdef RE_NO_NEWARRAY
                op->nbracket.name=static_cast<rechar*>(malloc(sizeof(rechar)*(len+1)));
                #else
                op->nbracket.name=new rechar[len+1];
                #endif
                memcpy(op->nbracket.name,src+i,len);
                op->nbracket.name[len]=0;
                //h.SetItem((char*)op->nbracket.name,m);
              }else
              {
                op->op=opOpenBracket;
                op->bracket.index=-1;
              }
              i+=len;
            }break;
#endif
            default:
            {
              return SetError(errSyntax,i);
            }
          }

        }else
        {
          brcount++;
          closedbrackets[brcount]=false;
          op->bracket.index=brcount;
        }
        brackets[brdepth]=op;
        brdepth++;
        continue;
      }
      case ')':
      {
        lookbehind=0;
        op->op=opClosingBracket;
        brdepth--;
        while(brackets[brdepth]->op==opAlternative)
        {
          brackets[brdepth]->alternative.endindex=op;
          brdepth--;
        }
        switch(brackets[brdepth]->op)
        {
          case opOpenBracket:
          {
            op->bracket.pairindex=brackets[brdepth];
            brackets[brdepth]->bracket.pairindex=op;
            op->bracket.index=brackets[brdepth]->bracket.index;
            if(op->bracket.index!=-1)
            {
              closedbrackets[op->bracket.index]=true;
            }
            break;
          }
#ifdef NAMEDBRACKETS
          case opNamedBracket:
          {
            op->nbracket.pairindex=brackets[brdepth];
            brackets[brdepth]->nbracket.pairindex=op;
            op->nbracket.name=brackets[brdepth]->nbracket.name;
            h.SetItem((char*)op->nbracket.name,m);
            break;
          }
#endif
          case opLookAhead:
          case opNotLookAhead:
          case opLookBehind:
          case opNotLookBehind:
          {
            op->assert.pairindex=brackets[brdepth];
            brackets[brdepth]->assert.pairindex=op;
            int l=0;
            PREOpCode p=brackets[brdepth];
            while(p!=op)
            {
              p=p->next;
              l++;
            }
            brackets[brdepth]->assert.length=l-1;
            break;
          }
        }
        continue;
      }
      case '[':
      {
        i++;
        int negative=0;
        if(src[i]=='^')
        {
          negative=1;
          i++;
        }
        int lastchar=0;
        int classsize=0;
        op->op=opSymbolClass;
        //op->symbolclass=new rechar[32];
        //memset(op->symbolclass,0,32);
        for(j=0;j<8;j++)itmpclass[j]=0;
        int classindex;
        for(;src[i]!=']';i++)
        {
          if(src[i]=='\\')
          {
            i++;
            int isnottype=0;
            int type=0;
            lastchar=0;
            switch(src[i])
            {
              case 'D':isnottype=1;
              case 'd':type=TYPE_DIGITCHAR;classindex=0;break;
              case 'W':isnottype=1;
              case 'w':type=TYPE_WORDCHAR;classindex=64;break;
              case 'S':isnottype=1;
              case 's':type=TYPE_SPACECHAR;classindex=32;break;
              case 'L':isnottype=1;
              case 'l':type=TYPE_LOWCASE;classindex=96;break;
              case 'U':isnottype=1;
              case 'u':type=TYPE_UPCASE;classindex=128;break;
              case 'I':isnottype=1;
              case 'i':type=TYPE_ALPHACHAR;classindex=160;break;
              case 'x':
              {
                int c=0;
                i++;
                if(i>=srclength)return SetError(errSyntax,i-1);
                c=lc[src[i]];
                if((c>='0' && c<='9') ||
                   (c>='a' && c<='f'))
                {
                  c-='0';
                  if(c>9)c-='a'-'0'-10;
                  lastchar=c;
                  if(i+1<srclength)
                  {
                    c=lc[src[i+1]];
                    if((c>='0' && c<='9') ||
                       (c>='a' && c<='f'))
                    {
                      i++;
                      c-='0';
                      if(c>9)c-='a'-'0'-10;
                      lastchar<<=4;
                      lastchar|=c;
                    }
                  }
                  dpf(("Last char=%c(%02x)\n",lastchar,lastchar));
                }else return SetError(errSyntax,i);
                break;
              }
              default:
              {
                if(options&OP_STRICT && chartypes[src[i]]&TYPE_ALPHACHAR)
                {
                  return SetError(errInvalidEscape,i-1);
                }
                lastchar=src[i];
              }
            }
            if(type)
            {
              isnottype=isnottype?0xffffffff:0;
              int *b=(int*)(charbits+classindex);
              for(j=0;j<8;j++)
              {
                itmpclass[j]|=b[j]^isnottype;
              }
              classsize=257;
              //for(int j=0;j<32;j++)op->symbolclass[j]|=charbits[classindex+j]^isnottype;
              //classsize+=charsizes[classindex>>5];
              //int setbit;
              /*for(int j=0;j<256;j++)
              {
                setbit=(chartypes[j]^isnottype)&type;
                if(setbit)
                {
                  if(ignorecase)
                  {
                    SetBit(op->symbolclass,lc[j]);
                    SetBit(op->symbolclass,uc[j]);
                  }else
                  {
                    SetBit(op->symbolclass,j);
                  }
                  classsize++;
                }
              }*/
            }else
            {
              if(options&OP_IGNORECASE)
              {
                SetBit(tmpclass,lc[lastchar]);
                SetBit(tmpclass,uc[lastchar]);
              }else
              {
                SetBit(tmpclass,lastchar);
              }
              classsize++;
            }
            continue;
          }
          if(src[i]=='-')
          {
            if(lastchar && src[i+1]!=']')
            {
              int to=src[i+1];
              if(to=='\\')
              {
                to=src[i+2];
                if(to=='x')
                {
                  i+=2;
                  to=src[i+1];
                  if((to>='0' && to<='9') ||
                     (to>='a' && to<='f'))
                  {
                    to-='0';
                    if(to>9)to-='a'-'0'-10;
                    if(i+1<srclength)
                    {
                      int c=lc[src[i+2]];
                      if((c>='0' && c<='9') ||
                         (c>='a' && c<='f'))
                      {
                        i++;
                        c-='0';
                        if(c>9)c-='a'-'0'-10;
                        to<<=4;
                        to|=c;
                      }
                    }
                  }else return SetError(errSyntax,i);
                }else
                {
                  SetBit(tmpclass,'-');
                  classsize++;
                  continue;
                }
              }
              i++;
              dpf(("from %d to %d\n",lastchar,to));
              for(j=lastchar;j<=to;j++)
              {
                if(ignorecase)
                {
                  SetBit(tmpclass,lc[j]);
                  SetBit(tmpclass,uc[j]);
                }else
                {
                  SetBit(tmpclass,j);
                }
                classsize++;
              }
              continue;
            }
          }
          lastchar=src[i];
          if(ignorecase)
          {
            SetBit(tmpclass,lc[lastchar]);
            SetBit(tmpclass,uc[lastchar]);
          }else
          {
            SetBit(tmpclass,lastchar);
          }
          classsize++;
        }
        if(negative && classsize>1)
        {
          for(int j=0;j<8;j++)itmpclass[j]^=0xffffffff;
          //for(int j=0;j<32;j++)op->symbolclass[j]^=0xff;
        }
        if(classsize==1)
        {
          op->op=negative?opNotSymbol:opSymbol;
          if(ignorecase)
          {
            op->op+=2;
            op->symbol=lc[lastchar];
          }
          else
          {
            op->symbol=lastchar;
          }
        }else
        if(classsize==256 && !negative)
        {
          op->op=options&OP_SINGLELINE?opCharAnyAll:opCharAny;
        }else
        {
          #ifdef RE_NO_NEWARRAY
          op->symbolclass=static_cast<rechar*>(malloc(sizeof(rechar)*32));
          #else
          op->symbolclass=new rechar[32];
          #endif
          for(j=0;j<8;j++)((int*)op->symbolclass)[j]=itmpclass[j];
        }
        continue;
      }
      case '+':
      case '*':
      case '?':
      case '{':
      {
        if(lookbehind)return SetError(errSyntax,i);
        int min,max;
        switch(src[i])
        {
          case '+':min=1;max=-1;break;
          case '*':min=0;max=-1;break;
          case '?':
          {
            //if(src[i+1]=='?') return SetError(errInvalidQuantifiersCombination,i);
            min=0;max=1;
            break;
          }
          case '{':
          {
            i++;
            int save=i;
            min=GetNum(src,i);
            max=min;
            if(min<0)return SetError(errInvalidRange,save);
//            i++;
            if(src[i]==',')
            {
              if(src[i+1]=='}')
              {
                i++;
                max=-1;
              }else
              {
                i++;
                max=GetNum(src,i);
//                i++;
                if(max<min)return SetError(errInvalidRange,save);
              }
            }
            if(src[i]!='}')return SetError(errInvalidRange,save);
          }
        }
        if(min==1 && max==1)continue;
        pos--;
        op=code+pos-1;
        op->range.min=min;
        op->range.max=max;
        switch(op->op)
        {
          case opLineStart:
          case opLineEnd:
          case opDataStart:
          case opDataEnd:
          case opWordBound:
          case opNotWordBound:
          {
            return SetError(errInvalidQuantifiersCombination,i);
//            op->range.op=op->op;
//            op->op=opRange;
//            continue;
          }
          case opCharAny:
          case opCharAnyAll:
          {
            op->range.op=op->op;
            op->op=opAnyRange;
            break;
          }
          case opType:
          {
            op->op=opTypeRange;
            break;
          }
          case opNotType:
          {
            op->op=opNotTypeRange;
            break;
          }
          case opSymbolIgnoreCase:
          case opSymbol:
          {
            op->op=opSymbolRange;
            break;
          }
          case opNotSymbol:
          case opNotSymbolIgnoreCase:
          {
            op->op=opNotSymbolRange;
            break;
          }
          case opSymbolClass:
          {
            op->op=opClassRange;
            break;
          }
          case opBackRef:
          {
            op->op=opBackRefRange;
            break;
          }
#ifdef NAMEDBRACKETS
          case opNamedBackRef:
          {
            op->op=opNamedRefRange;
          }break;
#endif
          case opClosingBracket:
          {
            op=op->bracket.pairindex;
            if(op->op!=opOpenBracket)return SetError(errInvalidQuantifiersCombination,i);
            op->range.min=min;
            op->range.max=max;
            op->op=opBracketRange;
            break;
          }
          default:
          {
            dpf(("OP=%d\n",op->op));
            return SetError(errInvalidQuantifiersCombination,i);
          }
        }//switch(code.op)
        if(src[i+1]=='?')
        {
           op->op++;
           i++;
        }
        continue;
      }// case +*?{
      case ' ':
      case '\t':
      case '\n':
      case '\r':
      {
        if(options&OP_XTENDEDSYNTAX)
        {
          pos--;
          continue;
        }
      }
#ifdef RELIB
      case '%':
      {
        i++;
        int len=0;
        while(src[i+len]!='%')len++;
        op->op=opLibCall;
        #ifdef RE_NO_NEWARRAY
        op->rename=static_cast<rechar*>(malloc(sizeof(rechar)*(len+1)));
        #else
        op->rename=new rechar[len+1];
        #endif
        memcpy(op->rename,src+i,len);
        op->rename[len]=0;
        i+=len;
        continue;
      }
#endif
      default:
      {
        op->op=options&OP_IGNORECASE?opSymbolIgnoreCase:opSymbol;
        if(ignorecase)
        {
          op->symbol=lc[src[i]];
        }else
        {
          op->symbol=src[i];
        }
      }
    }//switch(src[i])
  }//for()

  op=code+pos;
  pos++;

  brdepth--;
  while(brdepth>=0 && brackets[brdepth]->op==opAlternative)
  {
    brackets[brdepth]->alternative.endindex=op;
    brdepth--;
  }

  op->op=opClosingBracket;
  op->bracket.pairindex=code;
  code->bracket.pairindex=op;
#ifdef _DEBUG
  op->srcpos=i;
#endif

  op=code+pos;
  //pos++;
  op->op=opRegExpEnd;
#ifdef _DEBUG
  op->srcpos=i+1;
#endif

  return 1;
}

inline void RegExp::PushState()
{
  stackcount++;
#ifdef RELIB
  stackusage++;
#endif
  if(stackcount==STACK_PAGE_SIZE)
  {
    if(lastpage->next)
    {
      lastpage=lastpage->next;
      stack=lastpage->stack;
    }else
    {
      lastpage->next=new StateStackPage;
      lastpage->next->prev=lastpage;
      lastpage=lastpage->next;
      lastpage->next=NULL;
      #ifdef RE_NO_NEWARRAY
        lastpage->stack=static_cast<StateStackItem*>
         (CreateArray(sizeof(StateStackItem), STACK_PAGE_SIZE,
          StateStackItem::OnCreate));
      #else
        lastpage->stack=new StateStackItem[STACK_PAGE_SIZE];
      #endif // RE_NO_NEWARRAY
      stack=lastpage->stack;
    }
    stackcount=0;
  }
  st=&stack[stackcount];
}
int inline RegExp::PopState()
{
  stackcount--;
#ifdef RELIB
  stackusage--;
  if(stackusage<0)return 0;
#endif

  if(stackcount<0)
  {
    if(lastpage->prev==NULL)return 0;
    lastpage=lastpage->prev;
    stack=lastpage->stack;
    stackcount=STACK_PAGE_SIZE-1;
  }
  st=&stack[stackcount];
  return 1;
}


inline StateStackItem *RegExp::GetState()
{
  int tempcount=stackcount;
#ifdef RELIB
  if(stackusage==0)return 0;
#endif
  StateStackPage* temppage=lastpage;
  StateStackItem* tempstack=lastpage->stack;
  tempcount--;
  if(tempcount<0)
  {
    if(temppage->prev==NULL)return 0;
    temppage=temppage->prev;
  tempstack=temppage->stack;
    tempcount=STACK_PAGE_SIZE-1;
  }
  return &tempstack[tempcount];
}

inline StateStackItem *RegExp::FindStateByPos(PREOpCode pos,int op)
{
#ifdef RELIB
  int tempusage=stackusage;
#endif
  int tempcount=stackcount;
  StateStackPage* temppage=lastpage;
  StateStackItem* tempstack=lastpage->stack;
  do{
    tempcount--;
#ifdef RELIB
    tempusage--;
    if(tempusage<0)return 0;
#endif
    if(tempcount<0)
    {
      if(temppage->prev==NULL)return 0;
      temppage=temppage->prev;
      tempstack=temppage->stack;
      tempcount=STACK_PAGE_SIZE-1;
    }
  }while(tempstack[tempcount].pos!=pos || tempstack[tempcount].op!=op);
  return &tempstack[tempcount];
}


inline int RegExp::StrCmp(prechar& str,prechar start,prechar end)
{
  prechar save=str;
  if(ignorecase)
  {
    while(start<end)
    {
      if(lc[*str]!=lc[*start]){str=save;return 0;}
      str++;
      start++;
    }
  }else
  {
    while(start<end)
    {
      if(*str!=*start){str=save;return 0;}
      str++;
      start++;
    }
  }
  return 1;
}

#define OP (*op)


#define MINSKIP(cmp) \
            switch(op->next->op) \
            { \
              case opSymbol: \
              { \
                j=op->next->symbol; \
                if(*str!=j) \
                while(str<end && cmp && str[1]!=j && st->max--!=0)str++; \
                break; \
              } \
              case opNotSymbol: \
              { \
                j=op->next->symbol; \
                if(*str==j) \
                while(str<end && cmp && str[1]==j && st->max--!=0)str++; \
                break; \
              } \
              case opSymbolIgnoreCase: \
              { \
                j=op->next->symbol; \
                if(lc[str[1]]!=j) \
                while(str<end && cmp && lc[str[1]]!=j && st->max--!=0)str++; \
                break; \
              } \
              case opNotSymbolIgnoreCase: \
              { \
                j=op->next->symbol; \
                if(lc[str[1]]==j) \
                while(str<end && cmp && lc[str[1]]==j && st->max--!=0)str++; \
                break; \
              } \
              case opType: \
              { \
                j=op->next->type; \
                if(!(chartypes[str[1]]&j)) \
                while(str<end && cmp && !(chartypes[str[1]]&j) && st->max--!=0)str++; \
                break; \
              } \
              case opNotType: \
              { \
                j=op->next->type; \
                if((chartypes[str[1]]&j)) \
                while(str<end && cmp && (chartypes[str[1]]&j) && st->max--!=0)str++; \
                break; \
              } \
              case opSymbolClass: \
              { \
                cl=op->next->symbolclass; \
                if(!GetBit(cl,str[1])) \
                while(str<end && cmp && !GetBit(cl,str[1]) && st->max--!=0)str++; \
                break; \
              } \
            }

#ifdef RELIB
static void KillMatchList(MatchList *ml)
{
  int i;
  for(i=0;i<ml->Count();i++)
  {
    KillMatchList((*ml)[i].sublist);
    (*ml)[i].sublist=NULL;
  }
  ml->Clean();
}

#endif


int RegExp::InnerMatch(prechar str,const prechar end,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
  ,PMatchHash hmatch
#endif
)
{
//  register prechar str=start;
  int i,j;
  int minimizing;
  PREOpCode op,tmp;
  PMatch m;
  prechar cl;
#ifdef RELIB
  SMatchListItem ml;
#endif
  int inrangebracket=0;
  if(errorcode==errNotCompiled)return 0;
  if(matchcount<maxbackref)return SetError(errNotEnoughMatches,maxbackref);
#ifdef NAMEDBRACKETS
  if(havenamedbrackets && !hmatch)return SetError(errNoStorageForNB,0);
#endif
#ifdef RELIB
  if(reclevel<=1)
  {
#endif
    stackcount=0;
    lastpage=firstpage;
    stack=lastpage->stack;
    st=&stack[0];
#ifdef RELIB
  }
#endif
  StateStackItem *ps;
  errorcode=errNone;
  /*for(i=0;i<matchcount;i++)
  {
    match[i].start=-1;
    match[i].end=-1;
  }*/
  if(bracketscount<matchcount)matchcount=bracketscount;
  memset(match,-1,sizeof(*match)*matchcount);

  for(op=code;op;op=op->next)
  {
    //dpf(("op:%s,\tpos:%d,\tstr:%d\n",ops[OP.op],pos,str-start));
    dpf(("=================\n"));
    dpf(("S:%s\n%*s\n",start,str-start+3,"^"));
    dpf(("R:%s\n%*s\n",resrc,OP.srcpos+3,"^"));
    if(str<=end)
    switch(OP.op)
    {
      case opLineStart:
      {
        if(str==start || str[-1]==0x0d || str[-1]==0x0a)continue;
        break;
      }
      case opLineEnd:
      {
        if(str==end)continue;
        if(str[0]==0x0d || str[0]==0x0a)
        {
          if(str[0]==0x0d)str++;
          if(str[0]==0x0a)str++;
          continue;
        }
        break;
      }
      case opDataStart:
      {
        if(str==start)continue;
        break;
      }
      case opDataEnd:
      {
        if(str==end)continue;
        break;
      }
      case opWordBound:
      {
        if((str==start && chartypes[*str]&TYPE_WORDCHAR)||
           (!(chartypes[str[-1]]&TYPE_WORDCHAR) && chartypes[*str]&TYPE_WORDCHAR) ||
           (!(chartypes[*str]&TYPE_WORDCHAR) && chartypes[str[-1]]&TYPE_WORDCHAR) ||
           (str==end && chartypes[str[-1]]&TYPE_WORDCHAR))continue;
        break;
      }
      case opNotWordBound:
      {
        if(!((str==start && chartypes[*str]&TYPE_WORDCHAR)||
           (!(chartypes[str[-1]]&TYPE_WORDCHAR) && chartypes[*str]&TYPE_WORDCHAR) ||
           (!(chartypes[*str]&TYPE_WORDCHAR) && chartypes[str[-1]]&TYPE_WORDCHAR) ||
           (str==end && chartypes[str[-1]]&TYPE_WORDCHAR)))continue;
        break;
      }
      case opType:
      {
        if(chartypes[*str]&OP.type)
        {
          str++;
          continue;
        }
        break;
      }
      case opNotType:
      {
        if(!(chartypes[*str]&OP.type))
        {
          str++;
          continue;
        }
        break;
      }
      case opCharAny:
      {
        if(*str!=0x0d && *str!=0x0a)
        {
          str++;
          continue;
        }
        break;
      }
      case opCharAnyAll:
      {
        str++;
        continue;
      }
      case opSymbol:
      {
        if(*str==OP.symbol)
        {
          str++;
          continue;
        }
        break;
      }
      case opNotSymbol:
      {
        if(*str!=OP.symbol)
        {
          str++;
          continue;
        }
        break;
      }
      case opSymbolIgnoreCase:
      {
        if(lc[*str]==OP.symbol)
        {
          str++;
          continue;
        }
        break;
      }
      case opNotSymbolIgnoreCase:
      {
        if(lc[*str]!=OP.symbol)
        {
          str++;
          continue;
        }
        break;
      }
      case opSymbolClass:
      {
        if(GetBit(OP.symbolclass,*str))
        {
          str++;
          continue;
        }
        break;
      }
      case opOpenBracket:
      {
        if(OP.bracket.index>=0 && OP.bracket.index<matchcount)
        {
          if(inrangebracket)
          {
            st->op=opOpenBracket;
            st->pos=op;
            st->min=match[OP.bracket.index].start;
            st->max=match[OP.bracket.index].end;
            PushState();
          }
          match[OP.bracket.index].start=str-start;
        }
        if(OP.bracket.nextalt)
        {
          st->op=opAlternative;
          st->pos=OP.bracket.nextalt;
          st->savestr=str;
          PushState();
        }
        continue;
      }
#ifdef NAMEDBRACKETS
      case opNamedBracket:
      {
        if(hmatch)
        {
          PMatch m;
          if(!hmatch->Exists((char*)OP.nbracket.name))
          {
            tag_Match sm;
            sm.start=-1;
            sm.end=-1;
            m=hmatch->SetItem((char*)OP.nbracket.name,sm);
          }else
          {
            m=hmatch->GetPtr((char*)OP.nbracket.name);
          }
          if(inrangebracket)
          {
            st->op=opNamedBracket;
            st->pos=op;
            st->min=m->start;
            st->max=m->end;
            PushState();
          }
          m->start=str-start;
        }
        if(OP.bracket.nextalt)
        {
          st->op=opAlternative;
          st->pos=OP.bracket.nextalt;
          st->savestr=str;
          PushState();
        }
        continue;
      }
#endif
      case opClosingBracket:
      {
        switch(OP.bracket.pairindex->op)
        {
          case opOpenBracket:
          {
            if(OP.bracket.index>=0 && OP.bracket.index<matchcount)
            {
              match[OP.bracket.index].end=str-start;
            }
            continue;
          }
#ifdef NAMEDBRACKETS
          case opNamedBracket:
          {
            if(hmatch)
            {
              PMatch m=hmatch->GetPtr((char*)OP.nbracket.name);
              m->end=str-start;
            }
            continue;
          }
#endif
          case opBracketRange:
          {
            ps=FindStateByPos(OP.bracket.pairindex,opBracketRange);
            *st=*ps;
            if(str==st->startstr)
            {
              if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
              {
                match[OP.range.bracket.index].end=str-start;
              }
              inrangebracket--;
              continue;
            }
            if(st->min>0)st->min--;
            if(st->min)
            {
              st->max--;
              st->startstr=str;
              st->savestr=str;
              op=st->pos;
              PushState();
              if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
              {
                match[OP.range.bracket.index].start=str-start;
                st->op=opOpenBracket;
                st->pos=op;
                st->min=match[OP.range.bracket.index].start;
                st->max=match[OP.range.bracket.index].end;
                PushState();
              }
              if(OP.range.bracket.nextalt)
              {
                st->op=opAlternative;
                st->pos=OP.range.bracket.nextalt;
                st->savestr=str;
                PushState();
              }
              continue;
            }
            st->max--;
            if(st->max==0)
            {
              if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
              {
                match[OP.range.bracket.index].end=str-start;
              }
              inrangebracket--;
              continue;
            }
            if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
            {
              match[OP.range.bracket.index].end=str-start;
              tmp=op;
            }
            st->startstr=str;
            st->savestr=str;
            op=st->pos;
            PushState();
            if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
            {
              st->op=opOpenBracket;
              st->pos=tmp;
              st->min=match[OP.range.bracket.index].start;
              st->max=match[OP.range.bracket.index].end;
              PushState();
              match[OP.range.bracket.index].start=str-start;
            }

            if(OP.range.bracket.nextalt)
            {
              st->op=opAlternative;
              st->pos=OP.range.bracket.nextalt;
              st->savestr=str;
              PushState();
            }
            continue;
          }
          case opBracketMinRange:
          {
            ps=FindStateByPos(OP.bracket.pairindex,opBracketMinRange);
            *st=*ps;
            if(st->min>0)st->min--;
            if(st->min)
            {
              //st->min--;
              st->max--;
              st->startstr=str;
              st->savestr=str;
              op=st->pos;
              PushState();
              if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
              {
                match[OP.range.bracket.index].start=str-start;
                st->op=opOpenBracket;
                st->pos=op;
                st->min=match[OP.range.bracket.index].start;
                st->max=match[OP.range.bracket.index].end;
                PushState();
              }
              if(OP.range.bracket.nextalt)
              {
                st->op=opAlternative;
                st->pos=OP.range.bracket.nextalt;
                st->savestr=str;
                PushState();
              }
              continue;
            }
            if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
            {
              match[OP.range.bracket.index].end=str-start;
            }
            st->max--;
            inrangebracket--;
            if(st->max==0)continue;
            st->forward=str>ps->startstr?1:0;
            st->startstr=str;
            st->savestr=str;
            PushState();
            if(OP.range.bracket.nextalt)
            {
              st->op=opAlternative;
              st->pos=OP.range.bracket.nextalt;
              st->savestr=str;
              PushState();
            }
            continue;
          }
          case opLookAhead:
          {
            tmp=OP.bracket.pairindex;
            do{
              PopState();
            }while(st->pos!=tmp || st->op!=opLookAhead);
            str=st->savestr;
            continue;
          }
          case opNotLookAhead:
          {
            do{
              PopState();
            }while(st->op!=opNotLookAhead);
            str=st->savestr;
            break;
          }
          case opLookBehind:
          {
            continue;
          }
          case opNotLookBehind:
          {
            ps=GetState();
            ps->forward=0;
            break;
          }
        }//switch(code[pairindex].op)
        break;
      }//case opClosingBracket
      case opAlternative:
      {
        op=OP.alternative.endindex->prev;
        continue;
      }
#ifdef NAMEDBRACKETS
      case opNamedBackRef:
#endif
      case opBackRef:
      {
#ifdef NAMEDBRACKETS
        if(OP.op==opNamedBackRef)
        {
          if(!hmatch || !hmatch->Exists((char*)OP.refname))break;
          m=hmatch->GetPtr((char*)OP.refname);
        }else
        {
          m=&match[OP.refindex];
        }
#else
        m=&match[OP.refindex];
#endif
        if(m->start==-1 || m->end==-1)break;
        if(ignorecase)
        {
          j=m->end;
          for(i=m->start;i<j;i++,str++)
          {
            if(lc[start[i]]!=lc[*str])break;
            if(str>end)break;
          }
          if(i<j)break;
        }else
        {
          j=m->end;
          for(i=m->start;i<j;i++,str++)
          {
            if(start[i]!=*str)break;
            if(str>end)break;
          }
          if(i<j)break;
        }
        continue;
      }
      case opAnyRange:
      case opAnyMinRange:
      {
        st->op=OP.op;
        minimizing=OP.op==opAnyMinRange;
        j=OP.range.min;
        st->max=OP.range.max-j;
        if(OP.range.op==opCharAny)
        {
          for(i=0;i<j;i++,str++)
          {
            if(str>end || *str==0x0d || *str==0x0a)break;
          }
          if(i<j)
          {
            break;
          }
          st->startstr=str;
          if(!minimizing)
          {
            while(str<end && *str!=0x0d && *str!=0x0a && st->max--!=0)str++;
          }else
          {
            MINSKIP(*str!=0x0d && *str!=0x0a);
          }
        }else
        {
          //opCharAnyAll:
          str+=j;
          if(str>end)break;
          st->startstr=str;
          if(!minimizing)
          {
            str=end;
          }else
          {
            MINSKIP(1);
          }
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
      case opSymbolRange:
      case opSymbolMinRange:
      {
        st->op=OP.op;
        minimizing=OP.op==opSymbolMinRange;
        j=OP.range.min;
        st->max=OP.range.max-j;
        if(ignorecase)
        {
          for(i=0;i<j;i++,str++)
          {
            if(str>end || lc[*str]!=OP.range.symbol)break;
          }
          if(i<j)break;
          st->startstr=str;
          if(!minimizing)
          {
            while(str<end && lc[*str]==OP.range.symbol && st->max--!=0)str++;
          }else
          {
            MINSKIP(lc[*str]==OP.range.symbol);
          }
        }else
        {
          for(i=0;i<j;i++,str++)
          {
            if(str>end || *str!=OP.range.symbol)break;
          }
          if(i<j)break;
          st->startstr=str;
          if(!minimizing)
          {
            while(str<end && *str==OP.range.symbol && st->max--!=0)str++;
          }else
          {
            MINSKIP(*str==OP.range.symbol);
          }
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
      case opNotSymbolRange:
      case opNotSymbolMinRange:
      {
        st->op=OP.op;
        minimizing=OP.op==opNotSymbolMinRange;
        j=OP.range.min;
        st->max=OP.range.max-j;
        if(ignorecase)
        {
          for(i=0;i<j;i++,str++)
          {
            if(str>end || lc[*str]==OP.range.symbol)break;
          }
          if(i<j)break;
          st->startstr=str;
          if(!minimizing)
          {
            while(str<end && lc[*str]!=OP.range.symbol && st->max--!=0)str++;
          }else
          {
            MINSKIP(lc[*str]!=OP.range.symbol);
          }
        }else
        {
          for(i=0;i<j;i++,str++)
          {
            if(str>end || *str==OP.range.symbol)break;
          }
          if(i<j)break;
          st->startstr=str;
          if(!minimizing)
          {
            while(str<end && *str!=OP.range.symbol && st->max--!=0)str++;
          }else
          {
            MINSKIP(*str!=OP.range.symbol);
          }
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
      case opClassRange:
      case opClassMinRange:
      {
        st->op=OP.op;
        minimizing=OP.op==opClassMinRange;
        j=OP.range.min;
        st->max=OP.range.max-j;
        for(i=0;i<j;i++,str++)
        {
          if(str>end || !GetBit(OP.range.symbolclass,*str))break;
        }
        if(i<j)break;
        st->startstr=str;
        if(!minimizing)
        {
          while(str<end && GetBit(OP.range.symbolclass,*str) && st->max--!=0)str++;
        }else
        {
          MINSKIP(GetBit(OP.range.symbolclass,*str));
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
      case opTypeRange:
      case opTypeMinRange:
      {
        st->op=OP.op;
        minimizing=OP.op==opTypeMinRange;
        j=OP.range.min;
        st->max=OP.range.max-j;
        for(i=0;i<j;i++,str++)
        {
          if(str>end || (chartypes[*str]&OP.range.type)==0)break;
        }
        if(i<j)break;
        st->startstr=str;
        if(!minimizing)
        {
          while(str<end && (chartypes[*str]&OP.range.type) && st->max--!=0)str++;
        }else
        {
          MINSKIP((chartypes[*str]&OP.range.type));
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
      case opNotTypeRange:
      case opNotTypeMinRange:
      {
        st->op=OP.op;
        minimizing=OP.op==opNotTypeMinRange;
        j=OP.range.min;
        st->max=OP.range.max-j;
        for(i=0;i<j;i++,str++)
        {
          if(str>end || (chartypes[*str]&OP.range.type)!=0)break;
        }
        if(i<j)break;
        st->startstr=str;
        if(!minimizing)
        {
          while(str<end && (chartypes[*str]&OP.range.type)==0 && st->max--!=0)str++;
        }else
        {
          MINSKIP((chartypes[*str]&OP.range.type)==0);
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
#ifdef NAMEDBRACKETS
      case opNamedRefRange:
      case opNamedRefMinRange:
#endif
      case opBackRefRange:
      case opBackRefMinRange:
      {
        st->op=OP.op;
#ifdef NAMEDBRACKETS
        minimizing=OP.op==opBackRefMinRange || OP.op==opNamedRefMinRange;
#else
        minimizing=OP.op==opBackRefMinRange;
#endif
        j=OP.range.min;
        st->max=OP.range.max-j;
#ifdef NAMEDBRACKETS
        if(OP.op==opBackRefRange || OP.op==opBackRefMinRange)
        {
          m=&match[OP.range.refindex];
        }else
        {
          m=hmatch->GetPtr((char*)OP.range.refname);
        }
#else
        m=&match[OP.range.refindex];
#endif
        if(!m || m->start==-1 || m->end==-1)break;

        for(i=0;i<j;i++)
        {
          if(str>end || StrCmp(str,start+m->start,start+m->end)==0)break;
        }
        if(i<j)break;
        st->startstr=str;
        if(!minimizing)
        {
          while(str<end && StrCmp(str,start+m->start,start+m->end) && st->max--!=0);
        }else
        {
          MINSKIP(StrCmp(str,start+m->start,start+m->end));
        }
        if(OP.range.max==j)continue;
        st->savestr=str;
        st->pos=op;
        PushState();
        continue;
      }
      case opBracketRange:
      case opBracketMinRange:
      {
        if(inrangebracket && OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
        {
          st->op=opOpenBracket;
          st->pos=OP.range.bracket.pairindex;
          st->min=match[OP.range.bracket.index].start;
          st->max=match[OP.range.bracket.index].end;
          PushState();
        }
        st->op=OP.op;
        st->pos=op;
        st->min=OP.range.min;
        st->max=OP.range.max;
        st->startstr=str;
        st->savestr=str;
        st->forward=1;
        PushState();
        if(OP.range.nextalt)
        {
          st->op=opAlternative;
          st->pos=OP.range.bracket.nextalt;
          st->savestr=str;
          PushState();
        }
        if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
        {
          match[OP.range.bracket.index].start=
          /*match[OP.range.bracket.index].end=*/str-start;
        }
        if(OP.op==opBracketMinRange && OP.range.min==0)
        {
          op=OP.range.bracket.pairindex;
        }else
        {
          inrangebracket++;
        }
        continue;
      }
      case opLookAhead:
      case opNotLookAhead:
      {
        st->op=OP.op;
        st->savestr=str;
        st->pos=op;
        st->forward=1;
        PushState();
        if(OP.assert.nextalt)
        {
          st->op=opAlternative;
          st->pos=OP.assert.nextalt;
          st->savestr=str;
          PushState();
        }
        continue;
      }
      case opLookBehind:
      case opNotLookBehind:
      {
        st->op=OP.op;
        st->savestr=str;
        st->pos=op;
        st->forward=1;
        if(str-OP.assert.length<start)
        {
          if(OP.op==opLookBehind)break;
          continue;
        }
        str-=OP.assert.length;
        PushState();
        /*if(OP.assert.nextalt)
        {
          st->op=opAlternative;
          st->pos=OP.assert.nextalt;
          PushState();
        }*/
        continue;
      }
      case opNoReturn:
      {
        st->op=opNoReturn;
        PushState();
        continue;
      }
#ifdef RELIB
      case opLibCall:
      {
        if(!relib->Exists((char*)OP.rename))return 0;
/*        int ok=1;
        PMatchList curlist=matchlist;
        while(curlist)
        {
          if(curlist->Count()==0)
          {
            curlist=curlist->parent;
            continue;
          }
          int k=curlist->Count()-1;
          while(k>=0)
          {
            if((*curlist)[k].start!=str-start){ok=2;break;}
            if(!strcmp((char*)(*curlist)[k].name,(char*)OP.rename) && (*curlist)[k].pos==pos)
            {
              ok=0;
              break;
            }
            k--;
          }
          if(!ok || ok==2)break;
          if(k<0)curlist=curlist->parent;
        }
        if(!ok)
        {
          dpf(("Recursion detected! Declain call of %s\n",OP.rename));
          break;
        }
        */
        RegExp *re=(*relib)[(char*)OP.rename];
//        if(matchlist->parent && matchlist->parent->Count())
//        {
          prechar callfrom=matchlist->parent->Last().Get().name;
          int curpos=str-start;
          int k=re->cs.Count()-1;
          int ok=1;
          while(k>=0)
          {
            if(re->cs[k].strpos!=curpos)break;
            if(!strcmp((char*)re->cs[k].name,(char*)callfrom))
            {
              dpf(("Recursive call rejected: %s:%d\n",OP.rename,curpos));
              ok=0;
              break;
            }
            k--;
          }
          if(!ok)break;

//        }
        SCallStackItem csi;
        csi.name=callfrom;
        csi.strpos=curpos;
        re->cs.Append(csi);

        st->op=opLibCall;
        st->pos=op;
        st->startstr=OP.rename;
        st->savestr=str;
        st->cnt=matchlist->Count();
        PushState();
        SMatchListItem *pml;
        int res;
        if(re->havefirst && !re->first[*str])
        {
          dpf(("Fail first char check: %c\n",*str));
          res=0;
        }else
        {
          ml.name=OP.rename;
          ml.start=str-start;
          ml.sublist=new MatchList;
          ml.sublist->parent=matchlist;
          pml=&matchlist->Append(ml);

          SMatch mt[10];
          PMatch mtch=mt;
          if(re->bracketscount>10)
          #ifdef RE_NO_NEWARRAY
            mtch=static_cast<PMatch>(CreateArray(sizeof(SMatch),
              re->bracketscount, NULL));
          #else
            mtch=new SMatch[re->bracketscount];
          #endif // RE_NO_NEWARRAY
          MatchList *mls;
          mls=matchlist;
          re->SetMatchList(ml.sublist);
          int mcnt=re->bracketscount;
          int savecnt=re->stackcount;
          int savestack=re->stackusage;
          PStateStackPage savepage=re->lastpage;

          re->start=start;
          re->reclevel++;
          re->stackusage=0;
          dpf(("Call: %s\n",OP.rename));
#ifdef NAMEDBRACKETS
          MatchHash h;
#endif
          res=re->InnerMatch(str,end,mtch,mcnt
#ifdef NAMEDBRACKETS
                             ,&h
#endif
                            );
          dpf(("Return from: %s - %s\n",OP.rename,res?"ok":"fail"));
          re->cs.Pop(csi);
          matchlist=mls;
          re->reclevel--;
          re->stackusage=savestack;
          pml->end=mtch[0].end;
          if(mtch!=mt)
          #ifdef RE_NO_NEWARRAY
            DeleteArray(reinterpret_cast<void**>(&mtch),NULL);
          #else
            delete [] mtch;
          #endif // RE_NO_NEWARRAY
          re->lastpage=savepage;
          re->stackcount=savecnt;
          re->stack=re->lastpage->stack;
          re->st=&re->stack[re->stackcount];
          if(!res)
          {
            matchlist->Pop(ml);
            KillMatchList(ml.sublist);
          }
        }
        if(res)
        {
          str=start+pml->end;
        }else break;
        continue;
      }
#endif
      case opRegExpEnd:return 1;
    }//switch(op)
    for(;;PopState())
    {
      if(0==(ps=GetState()))return 0;
      dpf(("ps->op:%s\n",ops[ps->op]));
      switch(ps->op)
      {
        case opAlternative:
        {
          str=ps->savestr;
          op=ps->pos;
          if(OP.alternative.nextalt)
          {
            ps->pos=OP.alternative.nextalt;
          }else
          {
            PopState();
          }
          break;
        }
        case opAnyRange:
        case opSymbolRange:
        case opNotSymbolRange:
        case opClassRange:
        case opTypeRange:
        case opNotTypeRange:
        {
          str=ps->savestr-1;
          op=ps->pos;
          if(str<ps->startstr)
          {
            continue;
          }
          ps->savestr=str;
          break;
        }
#ifdef NAMEDBRACKETS
        case opNamedRefRange:
#endif
        case opBackRefRange:
        {
#ifdef NAMEDBRACKETS
//          PMatch m;
          if(ps->op==opBackRefRange)
          {
            m=&match[ps->pos->range.refindex];
          }else
          {
            m=hmatch->GetPtr((char*)ps->pos->range.refname);
          }
#else
          m=&match[ps->pos->range.refindex];
#endif
          str=ps->savestr-(m->end-m->start);
          op=ps->pos;
          if(str<ps->startstr)
          {
            continue;
          }
          ps->savestr=str;
          break;
        }
        case opAnyMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
          if(ps->pos->range.op==opCharAny)
          {
            if(str<end && *str!=0x0a && *str!=0x0d)
            {
              str++;
              ps->savestr=str;
            }else
            {
              continue;
            }
          }else
          {
            if(str<end)
            {
              str++;
              ps->savestr=str;
            }
            else
            {
              continue;
            }
          }
          break;
        }
        case opSymbolMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
          if(ignorecase)
          {
            if(str<end && lc[*str]==OP.symbol)
            {
              str++;
              ps->savestr=str;
            }else
            {
              continue;
            }
          }else
          {
            if(str<end && *str==OP.symbol)
            {
              str++;
              ps->savestr=str;
            }else
            {
              continue;
            }
          }
          break;
        }
        case opNotSymbolMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
          if(ignorecase)
          {
            if(str<end && lc[*str]!=OP.symbol)
            {
              str++;
              ps->savestr=str;
            }else
            {
              continue;
            }
          }else
          {
            if(str<end && *str!=OP.symbol)
            {
              str++;
              ps->savestr=str;
            }else
            {
              continue;
            }
          }
          break;
        }
        case opClassMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
          if(str<end && GetBit(OP.range.symbolclass,*str))
          {
            str++;
            ps->savestr=str;
          }else
          {
            continue;
          }
          break;
        }
        case opTypeMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
          if(str<end && chartypes[*str]&OP.range.type)
          {
            str++;
            ps->savestr=str;
          }else
          {
            continue;
          }
          break;
        }
        case opNotTypeMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
          if(str<end && ((chartypes[*str]&OP.range.type)==0))
          {
            str++;
            ps->savestr=str;
          }else
          {
            continue;
          }
          break;
        }
#ifdef NAMEDBRACKETS
        case opNamedRefMinRange:
#endif
        case opBackRefMinRange:
        {
          if(ps->max--==0)continue;
          str=ps->savestr;
          op=ps->pos;
#ifdef NAMEDBRACKETS
          if(ps->op==opBackRefMinRange)
          {
            m=&match[OP.range.refindex];
          }else
          {
            m=hmatch->GetPtr((char*)OP.range.refname);
          }
#else
          m=&match[OP.range.refindex];
#endif
          if(str+m->end-m->start<end && StrCmp(str,start+m->start,start+m->end))
          {
            ps->savestr=str;
          }else
          {
            continue;
          }
          break;
        }
        case opBracketRange:
        {
          if(ps->min)
          {
            inrangebracket--;
            continue;
          }
          if(ps->forward)
          {
            ps->forward=0;
            op=ps->pos->range.bracket.pairindex;
            inrangebracket--;
            str=ps->savestr;
//            if(OP.bracket.index>=0 && OP.bracket.index<matchcount)
//            {
//              match[OP.bracket.index].end=str-start;
//            }
            break;
          }
          continue;
        }
        case opBracketMinRange:
        {
          if(ps->max--==0)
          {
            inrangebracket--;
            continue;
          }
          if(ps->forward)
          {
            ps->forward=0;
            op=ps->pos;
            str=ps->savestr;
            if(OP.range.bracket.index>=0 && OP.range.bracket.index<matchcount)
            {
              match[OP.range.bracket.index].start=str-start;
              st->op=opOpenBracket;
              st->pos=op;
              st->min=match[OP.range.bracket.index].start;
              st->max=match[OP.range.bracket.index].end;
              PushState();
            }
            inrangebracket++;
            break;
          }
          inrangebracket--;
          continue;
        }
        case opOpenBracket:
        {
          j=ps->pos->bracket.index;
          if(j>=0 && j<matchcount)
          {
            match[j].start=ps->min;
            match[j].end=ps->max;
          }
          continue;
        }
#ifdef NAMEDBRACKETS
        case opNamedBracket:
        {
          prechar n=ps->pos->nbracket.name;
          if(n && hmatch)
          {
            SMatch sm;
            sm.start=ps->min;
            sm.end=ps->max;
            hmatch->SetItem((char*)n,sm);
          }
          continue;
        }
#endif
        case opLookAhead:
        case opLookBehind:
        {
          continue;
        }
        case opNotLookBehind:
        case opNotLookAhead:
        {
          op=ps->pos->assert.pairindex;
          str=ps->savestr;
          if(ps->forward)
          {
            PopState();
            break;
          }else
          {
            continue;
          }
        }
        case opNoReturn:
        {
          return 0;
        }
#ifdef RELIB
        case opLibCall:
        {
          op=ps->pos;
          str=ps->savestr;
          while(matchlist->Count()>ps->cnt)
          {
            matchlist->Pop(ml);
            KillMatchList(ml.sublist);
            ml.sublist=NULL;
          }
          //PopState();
          continue;
        }
#endif

     }//switch(op)
     break;
    }
  }
  return 1;
}

int RegExp::Match(const char* textstart,const char* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
  ,PMatchHash hmatch
#endif
)
{
  start=(const prechar)textstart;
  prechar tempend=(const prechar)textend;
  if(havefirst && !first[*start])return 0;
  TrimTail(tempend);
  if(tempend<start)return 0;
  int res=InnerMatch(start,tempend,match,matchcount
#ifdef NAMEDBRACKETS
  ,hmatch
#endif
  );
  if(res)
  {
    int i;
    for(i=0;i<matchcount;i++)
    {
      if(match[i].start==-1 || match[i].end==-1 || match[i].start>match[i].end)
      {
        match[i].start=match[i].end=-1;
      }
    }
  }
  return res;
}

int RegExp::MatchEx(const char* datastart,const char* textstart,const char* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
                 ,PMatchHash hmatch
#endif
  )
{
  if(havefirst && !first[(rechar)*textstart])return 0;
  prechar tempend=(const prechar)textend;
  if((prechar)datastart==start && (prechar)textend==end)
  {
    tempend=trimend;
  }else
  {
    start=(const prechar)datastart;
    TrimTail(tempend);
    trimend=tempend;
  }
  end=(const prechar)textend;
  if(tempend<(const prechar)textstart)return 0;
  int res=InnerMatch((const prechar)textstart,tempend,match,matchcount
#ifdef NAMEDBRACKETS
  ,hmatch
#endif
  );
  if(res)
  {
    int i;
    for(i=0;i<matchcount;i++)
    {
      if(match[i].start==-1 || match[i].end==-1 || match[i].start>match[i].end)
      {
        match[i].start=match[i].end=-1;
      }
    }
  }
  return res;
}


int RegExp::Match(const char* textstart,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
  ,PMatchHash hmatch
#endif
)
{
  const char* textend=textstart+strlen(textstart);
  return Match(textstart,textend,match,matchcount
#ifdef NAMEDBRACKETS
    ,hmatch
#endif
  );
}



int RegExp::Optimize()
{
  int i;
  PREOpCode jumps[MAXDEPTH];
  int jumpcount=0;
  if(havefirst)return 1;
  if(!code)return 0;
  memset(first,0,sizeof(first));
  PREOpCode op;
  for(op=code;;op=op->next)
  {
    switch(OP.op)
    {
      default:
      {
        return 0;
      }
      case opType:
      {
        for(i=0;i<255;i++)if(chartypes[i]&OP.type)first[i]=1;
        break;
      }
      case opNotType:
      {
        for(i=0;i<255;i++)if(!(chartypes[i]&OP.type))first[i]=1;
        break;
      }
      case opSymbol:
      {
        first[OP.symbol]=1;
        break;
      }
      case opSymbolIgnoreCase:
      {
        first[OP.symbol]=1;
        first[uc[OP.symbol]]=1;
        break;
      }
      case opSymbolClass:
      {
        for(i=0;i<256;i++)
        {
          if(GetBit(OP.symbolclass,i))first[i]=1;
        }
        break;
      }
#ifdef NAMEDBRACKETS
     case opNamedBracket:
#endif
      case opOpenBracket:
      {
        if(OP.bracket.nextalt)
        {
          jumps[jumpcount++]=OP.bracket.nextalt;
        }
        continue;
      }
      case opClosingBracket:
      {
        continue;
      }
      case opAlternative:
      {
        return 0;
      }
      case opSymbolRange:
      case opSymbolMinRange:
      {
        if(ignorecase)
        {
          first[lc[OP.range.symbol]]=1;
          first[uc[OP.range.symbol]]=1;
        }else
        {
          first[OP.range.symbol]=1;
        }
        if(OP.range.min==0)continue;
        break;
      }
      case opTypeRange:
      case opTypeMinRange:
      {
        for(i=0;i<256;i++)
        {
          if(chartypes[i]&OP.range.type)first[i]=1;
        }
        if(OP.range.min==0)continue;
        break;
      }
      case opNotTypeRange:
      case opNotTypeMinRange:
      {
        for(i=0;i<256;i++)
        {
          if(!(chartypes[i]&OP.range.type))first[i]=1;
        }
        if(OP.range.min==0)continue;
        break;
      }
      case opClassRange:
      case opClassMinRange:
      {
        for(i=0;i<256;i++)
        {
          if(GetBit(OP.range.symbolclass,i))first[i]=1;
        }
        if(OP.range.min==0)continue;
        break;
      }
      case opBracketRange:
      case opBracketMinRange:
      {
        if(OP.range.min==0)return 0;
        if(OP.range.bracket.nextalt)
        {
          jumps[jumpcount++]=OP.range.bracket.nextalt;
        }
        continue;
      }
      //case opLookAhead:
      //case opNotLookAhead:
      //case opLookBehind:
      //case opNotLookBehind:
      case opRegExpEnd:return 0;
    }
    if(jumpcount>0)
    {
      op=jumps[--jumpcount];
      continue;
    }
    break;
  }
  havefirst=1;
  return 1;
}

int RegExp::Search(const char* textstart,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
  ,PMatchHash hmatch
#endif
)
{
  const char* textend=textstart+strlen(textstart);
  return Search(textstart,textend,match,matchcount
#ifdef NAMEDBRACKETS
    ,hmatch
#endif
  );
}

int RegExp::Search(const char* textstart,const char* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
  ,PMatchHash hmatch
#endif
)
{
  start=(const prechar)textstart;
  prechar str=start;
  prechar tempend=(prechar)textend;
  TrimTail(tempend);
  if(tempend<start)return 0;
  if(code->bracket.nextalt==0 && code->next->op==opDataStart)
  {
    return InnerMatch(start,tempend,match,matchcount
#ifdef NAMEDBRACKETS
    ,hmatch
#endif
    );
  }
  if(havefirst)
  {
    do{
      while(!first[*str] && str<tempend)str++;
      if(InnerMatch(str,tempend,match,matchcount
#ifdef NAMEDBRACKETS
         ,hmatch
#endif
      ))return 1;
      str++;
    }while(str<tempend);
  }else
  {
    do{
      if(InnerMatch(str,tempend,match,matchcount
#ifdef NAMEDBRACKETS
         ,hmatch
#endif
      ))return 1;
      str++;
    }while(str<tempend);
  }
  return 0;
}

int RegExp::SearchEx(const char* datastart,const char* textstart,const char* textend,PMatch match,int& matchcount
#ifdef NAMEDBRACKETS
               ,PMatchHash hmatch
#endif
)
{
  start=(const prechar)datastart;
  prechar str=(prechar)textstart;
  prechar tempend=(prechar)textend;
  TrimTail(tempend);
  if(tempend<start)return 0;
  if(code->bracket.nextalt==0 && code->next->op==opDataStart)
  {
    return InnerMatch(start,tempend,match,matchcount
#ifdef NAMEDBRACKETS
    ,hmatch
#endif
    );
  }
  if(havefirst)
  {
    do{
      while(!first[*str] && str<tempend)str++;
      if(InnerMatch(str,tempend,match,matchcount
#ifdef NAMEDBRACKETS
         ,hmatch
#endif
      ))return 1;
      str++;
    }while(str<tempend);
  }else
  {
    do{
      if(InnerMatch(str,tempend,match,matchcount
#ifdef NAMEDBRACKETS
         ,hmatch
#endif
      ))return 1;
      str++;
    }while(str<tempend);
  }
  return 0;
}

void RegExp::TrimTail(prechar& end)
{
  if(havelookahead)return;
  if(!code || code->bracket.nextalt)return;
  PREOpCode op=code->bracket.pairindex->prev;
  while(OP.op==opClosingBracket)
  {
    if(OP.bracket.pairindex->op!=opOpenBracket)return;
    if(OP.bracket.pairindex->bracket.nextalt)return;
    op=op->prev;
  }
  end--;
  switch(OP.op)
  {
    case opSymbol:
    {
      while(end>=start && *end!=OP.symbol)end--;
      break;
    }
    case opNotSymbol:
    {
      while(end>=start && *end==OP.symbol)end--;
      break;
    }
    case opSymbolIgnoreCase:
    {
      while(end>=start && lc[*end]!=OP.symbol)end--;
      break;
    }
    case opNotSymbolIgnoreCase:
    {
      while(end>=start && lc[*end]==OP.symbol)end--;
      break;
    }
    case opType:
    {
      while(end>=start && !(chartypes[*end]&OP.type))end--;
      break;
    }
    case opNotType:
    {
      while(end>=start && chartypes[*end]&OP.type)end--;
      break;
    }
    case opSymbolClass:
    {
      while(end>=start && !GetBit(OP.symbolclass,*end))end--;
      break;
    }
    case opSymbolRange:
    case opSymbolMinRange:
    {
      if(OP.range.min==0)break;
      if(ignorecase)
      {
        while(end>=start && *end!=OP.range.symbol)end--;
      }else
      {
        while(end>=start && lc[*end]!=OP.range.symbol)end--;
      }
      break;
    }
    case opNotSymbolRange:
    case opNotSymbolMinRange:
    {
      if(OP.range.min==0)break;
      if(ignorecase)
      {
        while(end>=start && *end==OP.range.symbol)end--;
      }else
      {
        while(end>=start && lc[*end]==OP.range.symbol)end--;
      }
      break;
    }
    case opTypeRange:
    case opTypeMinRange:
    {
      if(OP.range.min==0)break;
      while(end>=start && !(chartypes[*end]&OP.range.type))end--;
      break;
    }
    case opNotTypeRange:
    case opNotTypeMinRange:
    {
      if(OP.range.min==0)break;
      while(end>=start && chartypes[*end]&OP.range.type)end--;
      break;
    }
    case opClassRange:
    case opClassMinRange:
    {
      if(OP.range.min==0)break;
      while(end>=start && !GetBit(OP.range.symbolclass,*end))end--;
      break;
    }
    default:break;
  }
  end++;
}

void RegExp::CleanStack()
{
  PStateStackPage tmp=firstpage->next,tmp2;
  while(tmp)
  {
    tmp2=tmp->next;
#ifdef RE_NO_NEWARRAY
      DeleteArray(reinterpret_cast<void**>(&tmp->stack),NULL);
#else
      delete [] tmp->stack;
#endif // RE_NO_NEWARRAY
    delete tmp;
    tmp=tmp2;
  }
}

void RegExp::SetLocaleInfo(prechar newlc,prechar newuc,prechar newchartypes
  #if defined(RE_EXTERNAL_CTYPE) && defined(RE_SPINOZA_MODE)
     , prechar newcharbits
  #endif
  )
{
#ifndef RE_EXTERNAL_CTYPE
  memcpy(lc,newlc,256);
  memcpy(uc,newuc,256);
  memcpy(chartypes,newchartypes,256);
#else
  lc=newlc;
  uc=newuc;
  chartypes=newchartypes;
  #ifdef RE_SPINOZA_MODE
    charbits=newcharbits;
  #endif
#endif
#ifndef RE_SPINOZA_MODE
  int i,j=0,k=1;
  memset(charbits,0,sizeof(charbits));
  for(i=0;i<256;i++)
  {
    if(chartypes[i]&TYPE_DIGITCHAR){charbits[j]|=k;}
    if(chartypes[i]&TYPE_SPACECHAR){charbits[32+j]|=k;}
    if(chartypes[i]&TYPE_WORDCHAR){charbits[64+j]|=k;}
    if(chartypes[i]&TYPE_LOWCASE){charbits[96+j]|=k;}
    if(chartypes[i]&TYPE_UPCASE){charbits[128+j]|=k;}
    if(chartypes[i]&TYPE_ALPHACHAR){charbits[160+j]|=k;}
    k<<=1;
    if(k==256){k=1;j++;}
  }
#endif
}

#ifdef RELIB

int RELibMatch(RELib& relib,MatchList& ml,const char* name,const char* start)
{
  return RELibMatch(relib,ml,name,start,start+strlen((char*)start));
}

int RELibMatch(RELib& relib,MatchList& ml,const char* name,const char* start,const char* end)
{
  char* k;
  RegExp *re;
  relib.First();
  while(relib.Next(k,re))
  {
    re->ResetRecursion();
  }
  if(!relib.Exists((char*)name))return 0;
  int cnt=relib[(char*)name]->GetBracketsCount();
  PMatch m=new SMatch[cnt];
  PMatchList pml=new MatchList;
  SMatchListItem li;
  li.name=(const prechar)name;
  li.sublist=pml;
  li.start=0;
  ml.Append(li);

  ml.parent=NULL;
  pml->parent=&ml;
  relib[(char*)name]->SetMatchList(pml);
#ifdef NAMEDBRACKETS
  MatchHash h;
#endif
  int res=relib[(char*)name]->Match(start,end,m,cnt
#ifdef NAMEDBRACKETS
  ,&h
#endif
  );
  ml.First().Get().start=m[0].start;
  ml.First().Get().end=m[0].end;

  delete [] m;
  return res;
}
#endif


};//regexp
};//util
};//smsc
