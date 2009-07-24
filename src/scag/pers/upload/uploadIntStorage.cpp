#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <stdlib.h>
#include <exception>
#include <memory>
#include <string>

#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "core/buffers/PageFile.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash64.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/util/io/HexDump.h"

#include "scag/pers/Glossary.h"
#include "scag/pers/Profile.h"
#include "scag/pers/Property.h"
#include "scag/pers/Types.h"
#include "scag/pers/VarRecSizeStore.h"
#include "scag/pers/upload/PersClient.h"

using namespace scag2::util::storage;
using namespace scag2::util;
using namespace scag::pers;
using namespace smsc::util::config;
using scag::pers::util::PersClient;
using scag::pers::util::PersClientException;
using std::string;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("pers");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        smsc_log_info(logger, "Stopping ...");
    }
    else if(sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

static const int MAX_PROP_IN_PACKET = 100;

int uploadProfile(smsc::logger::Logger* logger, const Profile& prof, uint32_t pkey, ProfileType type, PersClient* pc) {
  if (!pc) {
    smsc_log_warn(logger,"pers client is not inited");
    return 0;
  }
  const PropertyHash& phash = prof.getProperties();
  int propCount = phash.GetCount();
  int sentProperties = 0;
  int propRead = 0;
  try {
    smsc_log_debug(logger, "profile key=%d has %d properties", pkey, propCount);
    if (propCount == 0) {
      return 0;
    }
    PropertyHash::Iterator iter = phash.getIterator();
    int packetsCount = propCount % MAX_PROP_IN_PACKET == 0 ? propCount / MAX_PROP_IN_PACKET : propCount / MAX_PROP_IN_PACKET + 1;
    for (int i = 0; i < packetsCount; ++i) {
      SerialBuffer batch;
      pc->PrepareBatch(batch);
      char* key;
      Property* prop;
      int packetSize = 0;
      int propRead = 0;
      while ( propRead < MAX_PROP_IN_PACKET && iter.Next(key, prop) ) {
        if (prop->getTimePolicy() > UNKNOWN && prop->getTimePolicy() <= W_ACCESS) {
          pc->SetPropertyPrepare(type, pkey, *prop, batch);
          ++packetSize;
        }
        ++propRead;
      }
      if (packetSize == 0) {
        continue;
      }
      pc->FinishPrepareBatch(packetSize, batch);
      pc->RunBatch(batch);
      smsc_log_debug(logger, "send %d properties to pers for profile key=%d", packetSize, pkey);
      for (int j = 0; j < packetSize; ++j) {
        pc->SetPropertyResult(batch);
        ++sentProperties;
      }
    }
    if (sentProperties != propCount) {
      smsc_log_warn(logger, "profile key=%d not all properties sent: %d/%d has/sent", pkey, propCount, sentProperties);
    }
    return sentProperties > 0 ? 1 : 0;
  } catch (const SerialBufferOutOfBounds &e) {
    smsc_log_warn(logger, "Error uploading profile key=%d, %d/%d has/sent. SerialBufferOutOfBounds: bad data in buffer read ", pkey, propCount, sentProperties);
  } catch (const PersClientException& ex) {
    smsc_log_warn(logger, "Error uploading profile key=%d, %d/%d has/sent. PersClientException: %s", pkey, propCount, sentProperties, ex.what());
  }
  return 0;
}

void logoutProfile( smsc::logger::Logger* logger, const Profile& prof, const string& name) {
  const PropertyHash& phash = prof.getProperties();
  PropertyHash::Iterator iter(&phash);
  char* key;
  Property* value;
  int pcount = 0;
  smsc_log_info(logger,"%s: %s has %d properties", name.c_str(), prof.getKey().c_str(), phash.GetCount());
  while ( iter.Next(key,value) ) {
      smsc_log_info(logger," prop: %s", value->toString().c_str());
      ++pcount;
  }
}

void extractPageFile( smsc::logger::Logger* logger, PageFile& pf,
                      bool hasGlossary,
                      IntHash64< uint32_t >& indexHash,
                      ProfileType type, PersClient* pc, bool send , const string& name)
{
    File::offset_type pageIndex = 0;
    std::vector< unsigned char > data;
    File::offset_type realIndex;
    int readProfiles = 0;
    int uploadProfiles = 0;
    int notEmptyProfiles = 0;
    try {
        while ( true ) {

            File::offset_type nextIndex;
            bool corrupted = false;
            try {

                smsc_log_debug(logger,"=====================================================");
                nextIndex = pf.Read(pageIndex,data,&realIndex);

            } catch ( PageFileCorruptedException& e ) {

                corrupted = true;
                realIndex = e.initialPage();
                nextIndex = realIndex + pf.getPageSize();
                smsc_log_warn(logger,"corrupted: %s, skip to %lld", e.what(),
                              static_cast<long long>(nextIndex));
            }

            if ( ! nextIndex ) {
                smsc_log_debug(logger,"reading page=%lld gets EOF", static_cast<long long>(pageIndex) );
                break;
            }

            //Profile prof(logger);
            Profile prof;
            uint32_t pkey;
            IntProfileKey profileKey;
            bool hasProfileKey = false;
            {
                if ( ! indexHash.Get(realIndex,pkey) ) {
                    smsc_log_warn(logger,"profile key for page %lld not found",
                                  static_cast<long long>(realIndex));
                } else {
                    profileKey = IntProfileKey(pkey);
                    hasProfileKey = true;
                    prof.setKey(profileKey.toString());
                    smsc_log_debug(logger,"profile key: %s",prof.getKey().c_str());
                }
            }
/*
            std::string dump;
            {
                HexDump hd;
                unsigned dumpsize = std::min(unsigned(data.size()),unsigned(256));
                hd.hexdump(dump,(const char*)&data[0],dumpsize);
                dump += ": ";
                hd.strdump(dump,(const char*)&data[0],dumpsize);
            }
*/
            //smsc_log_debug(logger,"page=%lld real=%lld datasize=%ld key=%d data=%s",
            smsc_log_debug(logger,"page=%lld real=%lld datasize=%ld key=%d",
                          static_cast<long long>(pageIndex),
                          static_cast<long long>(realIndex),
                          long(data.size()),
                          pkey);
                          //dump.c_str() );
            pageIndex = nextIndex;

            SerialBuffer sb(data.size());
            sb.Append((char*)&data[0],data.size());
            sb.SetPos(0);
            try {
                prof.Deserialize( sb, hasGlossary );
            } catch ( std::exception& e ) {
                smsc_log_warn(logger,"deserialize profile key=%d exception: %s", pkey, e.what());
            }
            ++readProfiles;
            if (prof.getProperties().GetCount() > 0) {
              //if (type == PT_SERVICE && pkey != 135 && pkey != 140) {
                //continue;
              //}
              ++notEmptyProfiles;
            }
            if (send) {
              uploadProfiles += uploadProfile(logger, prof, pkey, type, pc);
            } else {
              logoutProfile(logger, prof, name);
            }

        }
    } catch ( std::exception& e ) {
        smsc_log_error(logger,"exception: %s", e.what());
    }
    //if (type == PT_SERVICE) {
      //smsc_log_info(logger,"read profiles: %d, matched profiles: %d, uploaded profiles: %d", readProfiles, notEmptyProfiles, uploadProfiles);
    //} else {
      smsc_log_info(logger,"read profiles: %d, not empty profiles: %d, uploaded profiles: %d", readProfiles, notEmptyProfiles, uploadProfiles);
    //}
}


struct OffsetValue{
    File::offset_type value;

    OffsetValue():value(0){}
    OffsetValue(File::offset_type argValue):value(argValue){}
    OffsetValue(const OffsetValue& src):value(src.value){}

    static uint32_t Size(){return sizeof(File::offset_type);}
    void Read(File& f)
    {
        value=f.ReadNetInt64();
    }
    void Write(File& f)const
    {
        f.WriteNetInt64(value);
    }
};


void extractDiskHash( smsc::logger::Logger* logger,
                      DiskHash< IntProfileKey, OffsetValue >& dh,
                      IntHash64< uint32_t >& indexHash )
{
    smsc_log_debug(logger,"=== extraction of disk hash data ===");
    DiskHash< IntProfileKey, OffsetValue >::Iterator iter(dh);
    IntProfileKey key;
    OffsetValue value;
    while ( iter.Next(key,value) ) {
        indexHash.Insert(value.value,*key.getKey());
        smsc_log_debug(logger,"key=%s value=%ld", key.toString().c_str(), long(value.value) );
    }
}


bool uploadStorage(ProfileType type, const string& storagePath, bool hasGlossary, smsc::logger::Logger* logger, PersClient* pc, bool send) {
  string storageName;
  switch (type) {
  case scag::pers::PT_OPERATOR : storageName = "operator"; break;
  case scag::pers::PT_PROVIDER : storageName = "provider"; break;
  case scag::pers::PT_SERVICE  : storageName = "service"; break; 
  default: smsc_log_error(logger, "unknown storage type: %d", type); return false;
  }
  smsc_log_info(logger, "=== read %s storage ===", storageName.c_str());
  std::string fn = storagePath + '/' + storageName;
  const std::string diskHashFilename = fn + ".idx";
  if ( ! File::Exists(diskHashFilename.c_str()) ) {
      smsc_log_error(logger, "disk hash does not exist at %s", diskHashFilename.c_str());
      return false;
  }

  const std::string pageFilename = fn + ".bin";
  if ( ! File::Exists(pageFilename.c_str()) ) {
      smsc_log_error(logger, "page file does not exist at %s", pageFilename.c_str());
      return false;
  }

  IntHash64< uint32_t > indexHash;
  {
      DiskHash< IntProfileKey, OffsetValue > dh;
      smsc_log_debug(logger,"opening diskhash file %s",diskHashFilename.c_str());
      dh.Open( diskHashFilename.c_str(), true, false );
      extractDiskHash( logger, dh, indexHash );
      smsc_log_debug(logger,"extracted %ld indices", long(indexHash.Count()));
  }

  {
      PageFile pf;
      smsc_log_debug(logger,"opening page file %s",pageFilename.c_str());
      pf.Open(pageFilename);
      extractPageFile( logger, pf, hasGlossary, indexHash, type, pc, send, storageName );
  }
  return true;
}

int main( int argc, const char** argv )
{

  Logger::Init();
  Logger* logger = Logger::getInstance("pers");

  atexit(atExitHandler);
  sigset_t set;    
  sigfillset(&set);
  sigdelset(&set, SIGTERM);
  sigdelset(&set, SIGINT);
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGBUS);
  sigdelset(&set, SIGHUP);
  sigprocmask(SIG_SETMASK, &set, NULL);
  sigset(SIGTERM, appSignalHandler);
  sigset(SIGINT, appSignalHandler);
  sigset(SIGHUP, appSignalHandler);    


    try {

      smsc_log_info(logger,  "Starting up Pers Infrastruct Storages Upload");
      Manager::init("config.xml");
      Manager& manager = Manager::getInstance();

      ConfigView uploadConfig(manager, "PersUpload");

      string storagePath = "./storage";
      try { 
        storagePath = uploadConfig.getString("storagePath");
      } catch (...) {
        smsc_log_warn(logger, "Parameter <PersUpload.AbntProfStorage.storagePath> missed. Defaul value is %d", storagePath.c_str());
      }

      bool sendToPers = false;
      try { sendToPers = uploadConfig.getBool("sendToPers"); } 
      catch (...) { 
          smsc_log_warn(logger, "Parameter <PersUpload.sendToPers> missed. Defaul value is false");
      }

      int timeOut = 1000;
      try { timeOut = uploadConfig.getInt("timeout"); } 
      catch (...) { 
          smsc_log_warn(logger, "Parameter <PersUpload.timeout> missed. Defaul value is %d", timeOut);
      }

      std::string host = "phoenix";
      try { 
        host = uploadConfig.getString("host");
      } catch (...) {
        smsc_log_warn(logger, "Parameter <PersUpload.host> missed. Defaul value is %s", host.c_str());
      }

      int port = 47880;
      try { 
        port = uploadConfig.getInt("port");
      } catch (...) {
        smsc_log_warn(logger, "Parameter <PersUpload.port> missed. Defaul value is %d", port);
      }

      int speed = 100;
      try { 
        speed = uploadConfig.getInt("speed");
      } catch (...) {
        smsc_log_warn(logger, "Parameter <PersUpload.speed> missed. Defaul value is %d", speed);
      }


      bool hasGlossary;
      {
          std::string glossfn = storagePath + "/glossary";
          hasGlossary = File::Exists(glossfn.c_str());
          if ( hasGlossary ) {
              smsc_log_info(logger,"opening glossary %s", glossfn.c_str());
              Glossary::Open(glossfn);
          } else {
              smsc_log_warn(logger,"glossary %s not found", glossfn.c_str());
          }
      }

      int pingTimeOut = 1000;
      int reconnectTimeout = 100;
      int maxWaitRequestsCount = 100;
      PersClient *pc = 0;
      if (sendToPers) {
        PersClient::Init(host.c_str(), port, timeOut, pingTimeOut, reconnectTimeout, maxWaitRequestsCount, speed);
        pc = &PersClient::Instance();
      }

      uploadStorage(PT_PROVIDER, storagePath, hasGlossary, logger, pc, sendToPers);
      uploadStorage(PT_OPERATOR, storagePath, hasGlossary, logger, pc, sendToPers);
      uploadStorage(PT_SERVICE, storagePath, hasGlossary, logger, pc, sendToPers);

    } catch (const PersClientException& exc) {
        smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
    } catch (const ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
    } catch ( std::exception& e ) {
        smsc_log_error(logger, "top level exception: %s", e.what());
    } catch (...) {
        smsc_log_error(logger, "unknown top level exception");
    }
    return 0;
}
