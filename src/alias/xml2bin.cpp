#include <stdio.h>
#include "util/config/alias/aliasconf.h"
#include "util/config/Manager.h"
#include "util/findConfigFile.h"
#include "alias/AliasManImpl.hpp"

using namespace smsc::util;

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  try{
    config::alias::AliasConfig aliascfg;
    config::Manager::init(findConfigFile("config.xml"));
    aliascfg.load(findConfigFile("aliases.xml"));
    smsc::alias::AliasManImpl am(config::Manager::getInstance().getString("aliasman.storeFile"));
    am.Load();

    config::alias::AliasConfig::RecordIterator i = aliascfg.getRecordIterator();
    config::alias::AliasRecord *rec;
    int cnt=0;
    while(i.hasRecord())
    {
      i.fetchNext(rec);
      smsc::alias::AliasInfo ai;
      ai.addr = smsc::sms::Address(
        strlen(rec->addrValue),
        rec->addrTni,
        rec->addrNpi,
        rec->addrValue);
      ai.alias = smsc::sms::Address(
        strlen(rec->aliasValue),
        rec->aliasTni,
        rec->aliasNpi,
        rec->aliasValue);
      ai.hide = rec->hide;
      try{
        am.addAlias(ai);
        cnt++;
      }catch(std::exception& e)
      {
        fprintf(stderr,"addAlias failed:%s\n",e.what());
      }
    }
    printf("%d records converted\n",cnt);
  }catch(std::exception& e)
  {
    fprintf(stderr,"Exception:%s\n",e.what());
  }
  return 0;
}
