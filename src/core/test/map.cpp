#include <stdio.h>
#include <map>
#include <string>
#include "core/buffers/Hash.hpp"

typedef std::map<std::string,int> IntMap;

void aaa(IntMap* m)
{
  (*m)["hello"]=0;
  (*m)["world"]=1;
}

int main(int argc,char* argv[])
{
  IntMap m;
  for(int i=0;i<10000;i++)
  {
    char buf[64];
    sprintf(buf,"key%d",i);
    m[buf]=i;
  }
  for(int i=0;i<10000;i++)
  {
    char buf[64];
    sprintf(buf,"key%d",i);
    m.erase(buf);
  }
  /*smsc::core::buffers::Hash<int> m;
  for(int i=0;i<10000;i++)
  {
    char buf[64];
    sprintf(buf,"key%d",i);
    m[buf]=i;
  }
  for(int i=0;i<10000;i++)
  {
    char buf[64];
    sprintf(buf,"key%d",i);
    m.Delete(buf);
  }*/
  return 0;
}
