#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "logger/Logger.h"

#include "mcisme/Profiler.h"
#include "mcisme/ProfilesStorage.hpp"

using namespace smsc::util::config;
using namespace smsc::db;
using namespace smsc::mcisme;
using namespace std;

int main(int argc, char** argv)
{
  Logger::Init();

  if ( argc < 4 ) {
    fprintf(stderr, "Usage: %s path_to_dir_with_profile_db abonents_hash_db on/off [file_with_abonents_list]\n", argv[0]);
    return 1;
  }
  bool on=false;
  try
  {
    ProfilesStorage::Open(argv[1], argv[2], false);

    AbonentProfile profile;
    ProfilesStorage* profStorage = ProfilesStorage::GetInstance();

    char abonent_isdn_string[32];
    FILE* fstream;
    if ( argc == 5 ) {
      fstream = fopen(argv[4], "r");
      if ( !fstream ) {
        fprintf(stderr, "can't open file [%s]: %s", argv[4], strerror(errno));
        return 1;
      }
    } else
      fstream = stdin;

    if ( !strcmp(argv[3], "on") )
      on = true;

    while(fgets(abonent_isdn_string, static_cast<int>(sizeof(abonent_isdn_string)),
                fstream))
    {
      abonent_isdn_string[strlen(abonent_isdn_string)-1] = 0;
      AbntAddr abonent(abonent_isdn_string);
      if ( profStorage->Get(abonent, profile) ) {
/*
        printf("dump of current profile for abonent '%s': eventMask=%02X,informTemplateId=%d,notifyTemplateId=%d,inform flag=%s,notify flag=%s,wantNotifyMe flag=%s\n",
               abonent_isdn_string, profile.eventMask, profile.informTemplateId,
               profile.notifyTemplateId,
               (profile.inform? "true" : "false"),
               (profile.notify ? "true" : "false"),
               (profile.wantNotifyMe ? "true" : "false"));
*/
        bool oldWantNotifyMeValue = profile.wantNotifyMe;
        if ( on )
          profile.wantNotifyMe = true;
        else
          profile.wantNotifyMe = false;
        if ( oldWantNotifyMeValue != profile.wantNotifyMe )
          profStorage->Set(abonent, profile);
      }
    }
    ProfilesStorage::Close();
  }
  catch(std::exception& ex)
  {
    fprintf(stderr, "caught exception %s", ex.what());
    try {
      ProfilesStorage::Close();
    } catch (...) {}
  }
  return 0;
}
