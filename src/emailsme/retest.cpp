#include <stdio.h>
#include <string.h>
#include "util/regexp/RegExp.hpp"

using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using smsc::core::buffers::Hash;

void TryToShow(const char* text,Hash<SMatch>& h,const char* key)
{
  if(!h.Exists(key))
  {
    printf("%s: <none>\n",key);
    return;
  }
  SMatch& m=h[key];
  if(m.start==-1 || m.end==-1)
  {
    printf("%s: <none>\n",key);
    return;
  }
  char buf[512];
  memcpy(buf,text+m.start,m.end-m.start);
  buf[m.end-m.start]=0;
  printf("%s:%s\n",key,buf);
}

int main(int argc,char* argv[])
{
  if(argc==1)
  {
    printf("Usage: retest \"emailsmesmstext\"\n");
    return -1;
  }
  RegExp re;
  RegExp::InitLocale();
  char *src=
"/(#(?{flag}\\w)#)?"                         // optional control flag
"(?{address}(?:.*?[@*][^#\\(\\s,]+,?)+)"        // mandatory address(es)
"(#(?{realname}[^#(]+))?"                    // optional real name
"(?:##(?{subj}.*?)#|\\((?{subj}.*?)\\)|\\s)" // optinal subject
"(?{body}.+)$/xs"                            // body
  ;
  if(!re.Compile(src))
  {
    printf("Compile failed:%d,%d\n",re.LastError(),re.ErrorPosition());
    printf("%s\n%*s\n",src,re.ErrorPosition(),"^");
    return -1;
  };
  Hash<SMatch> h;
  SMatch m[10];
  int n=10;
  char *str=argv[1];
  if(!re.Match(str,m,n,&h))
  {
    printf("Match failed\n");
    return -1;
  }else
  {
    TryToShow(str,h,"flag");
    TryToShow(str,h,"address");
    TryToShow(str,h,"realname");
    TryToShow(str,h,"subj");
    TryToShow(str,h,"body");
  }

  return 0;
}
