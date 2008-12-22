#ifndef ___BLOCKS_HS_READER_H
#define ___BLOCKS_HS_READER_H

#include "scag/pers/BlocksHSStorage.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pers/Profile.h"
#include "scag/pers/upload/PersClient.h"

namespace scag { namespace pers { namespace util {

  using scag::util::storage::SerialBuffer;
  using scag::util::storage::SerialBufferOutOfBounds;
  using namespace scag::pers;
  using scag::pers::util::PersClient;
  using scag::pers::util::PersClientException;
  using scag::util::storage::GlossaryBase;


  static const char* restore_properties[] = {
    "maze.abonent.age",
    "maze.abonent.gender",
    "maze.abonent.race.portal",
    "maze.abonent.level.portal",
    "maze.abonent.experience.portal",
    "maze.abonent.skill.attack.portal",
    "maze.abonent.skill.defence.portal",
    "maze.abonent.skill.quickness.portal",
    "maze.abonent.level.up.notified.portal",

    "maze.abonent.name",
    "maze.abonent.inviter",
    "maze.game.status",
    "maze.room.direction.1",
    "maze.room.direction.2",
    "maze.room.direction.3",
    "maze.room.description",
    "maze.last.enemy.1",
    "maze.last.enemy.2",
    "maze.last.enemy.3",
    "maze.last.enemy.4",
    "maze.last.enemy.5",

    "maze.room.searched",

    "maze.abonent.money.portal",
    "maze.abonent.money.portal.stage.2"
  };
  const int RESTORE_PROPERTIES_COUNT = 24;

  const char* INC_PROPERTY_NAME = "maze.abonent.money.portal";
  const char* STATUS_PROPERTY   = "maze.game.status";

const int BLOCK_SIZE = 2048;
const int BLOCKS_IN_FILE = 50000;

template<class Key>
class BlocksHSReader
{
public:
	typedef templDataBlockHeader<Key> DataBlockHeader;

    BlocksHSReader(PersClient& _pc, const string& _dbName, const string& _dbPath, int _blockSize = BLOCK_SIZE,
                    int _blocksInFile = BLOCKS_IN_FILE): pc(_pc), dbName(_dbName), dbPath(_dbPath),
                     blockSize(_blockSize), blocksInFile(_blocksInFile) { 
      logger = smsc::logger::Logger::getInstance("pers.up");
      smsc_log_debug(logger, "dbName='%s' dbPath='%s' blockSize=%d blocksInFile=%d",
                     dbName.c_str(), dbPath.c_str(), blockSize, blocksInFile);
    }

    ~BlocksHSReader() {
    }

    int OpenDataFiles(int files_count)
    {
        char	buff[10];
        string path = dbPath + '/' + dbName;
        for(long i = 0; i < files_count; i++)
        {
            snprintf(buff, 10, "-%.7d", i);
            string name = path + buff;
            dataFile_f.push_back(new File());
            try
            {
                smsc_log_debug(logger, "Open data file: %s", name.c_str());
                dataFile_f[i]->RWOpen(name.c_str());
                dataFile_f[i]->SetUnbuffered();
            }
            catch(FileException ex)
            {
                smsc_log_error(logger, "Cannot open data file: %s", ex.what());
                return CANNOT_OPEN_DATA_FILE;
            }
        }
        return 0;
    }
    
    int readDataFiles(int files_count, bool sendToPers) {
      if (!files_count) {
        return 0;
      }

      char	buff[10];
      string path = dbPath + '/' + dbName +'/' + dbName + "-data";
      long effectiveBlockSize = blockSize - sizeof(DataBlockHeader);
      char* data_buff;
      int total_count = 0;
      int total_status_profiles = 0;
      smsc_log_debug(logger, "start read %d files", files_count);

      for(long i = 0; i < files_count; i++)
      {
          snprintf(buff, 10, "-%.7d", i);
          string name = path + buff;
          try {
            int profiles_count = 0;
            int status_profiles = 0;
            smsc_log_debug(logger, "Open data file %d: %s", i + 1, name.c_str());
            File dataFile;
            dataFile.ROpen(name.c_str());
            dataFile.SetUnbuffered();
            DataBlockHeader hdr;
            for (int j = 0; j < blocksInFile; ++j) {
              //smsc_log_debug(logger, "seek pos = %d", j * blockSize);
              dataFile.Seek(j * blockSize, SEEK_SET);
              memset((void*)&hdr, 0, sizeof(DataBlockHeader));
              dataFile.Read((void*)&hdr, sizeof(DataBlockHeader));
              if (hdr.block_used != BLOCK_USED) {
                //smsc_log_debug(logger, "data block number=%d is not used", j);
                continue;
              }
              if (!hdr.head) {
                smsc_log_error(logger, "long data block number=%d pfkey=%s blocks:%d data size:%d",
                                j, hdr.key.toString().c_str(), hdr.total_blocks, hdr.data_size);
                continue;
              }
              ++profiles_count;
              smsc_log_debug(logger, "profile key=%s", hdr.key.toString().c_str());
              //smsc_log_debug(logger, "header data size=%d", hdr.data_size);

              SerialBuffer data;
              data.setBuffLength(hdr.data_size);
              //data.setBuffLength(effectiveBlockSize);
              data_buff = data.ptr();
              //data.setLength(effectiveBlockSize);
              data.setLength(hdr.data_size);
              //dataFile.Read((void*)data_buff, effectiveBlockSize);
              dataFile.Read((void*)data_buff, hdr.data_size);
              //status_profiles += restoreProfile(hdr.key, data, sendToPers);
              status_profiles += restoreProfileCompletely(hdr.key, data, sendToPers);
            }
            total_count += profiles_count;
            total_status_profiles += status_profiles;
            smsc_log_info(logger, "profiles count = %d/%d in file %d", profiles_count, total_count, i + 1);
            smsc_log_info(logger, "uploaded profiles count = %d/%d in file %d", status_profiles, total_status_profiles, i + 1);
            dataFile.Close();
          } catch (const FileException& ex) {
            smsc_log_error(logger, "Cannot open data file: %s", ex.what());
            return CANNOT_OPEN_DATA_FILE;
          }
      }
      smsc_log_info(logger, "total profiles count = %d in %d files", total_count, files_count);
      smsc_log_info(logger, "total uploaded profiles = %d", total_status_profiles);
      return 0;
    }
private:
  int restoreProfileCompletely(const Key& key, SerialBuffer& data, bool sendToPers) {
    try {
      Profile pf(key.toString());   
      pf.Deserialize(data, true);
      SerialBuffer batch;
      
      if (sendToPers) {
        pc.PrepareMTBatch(batch, PT_ABONENT, key.toString().c_str(), pf.GetCount(), true);
      }

      PropertyHash::Iterator it = pf.getProperties().getIterator();
      Property* prop;
      char *key = 0;
      int prop_count = 0;
      while(it.Next(key, prop)) {
        if (sendToPers) {
          pc.SetPropertyPrepare(*prop, batch);
        }
        smsc_log_debug(logger, "key=%s property=%s", pf.getKey().c_str(), prop->toString().c_str());
        ++prop_count;
      }
      if (sendToPers) {
        pc.RunBatch(batch);
        smsc_log_debug(logger, "send %d properties to pers for profile key=%s", prop_count, pf.getKey().c_str());
      }
      if (sendToPers) {
        for (int i = 0; i < prop_count; ++i) {
          pc.SetPropertyResult(batch);
        }
        return 1;
      }
    } catch (const SerialBufferOutOfBounds &e) {
      smsc_log_warn(logger, "SerialBufferOutOfBounds: bad data in buffer read. profile key=%s", key.toString().c_str());
    } catch (const PersClientException& ex) {
      smsc_log_warn(logger, "Error uploading profile key=%s. PersClientException: %s", key.toString().c_str(), ex.what());
    }
    return 0;
  }

  int restoreProfile(const Key& key, SerialBuffer& data, bool sendToPers) {
    try {
      Profile pf(key.toString());   
      pf.Deserialize(data, true);
      if (!pf.GetProperty(STATUS_PROPERTY)) {
        return 0;
      }
      int prop_count = 0;
      SerialBuffer batch;
      if (sendToPers) {
        pc.PrepareBatch(batch);
      }
      Property* prop = 0;
      /*if (prop = pf.GetProperty(INC_PROPERTY_NAME)) {
        if (sendToPers) {
          pc.IncPropertyPrepare(PT_ABONENT, key.toString().c_str(), *prop, batch);
        }
        ++prop_count;
      } else {
        smsc_log_debug(logger, "property %s not found in profile %s", INC_PROPERTY_NAME, pf.getKey().c_str());               
      }*/
      for (int i = 0; i < RESTORE_PROPERTIES_COUNT; ++i) {
        if (prop = pf.GetProperty(restore_properties[i])) {
          if (sendToPers) {
            pc.SetPropertyPrepare(PT_ABONENT, key.toString().c_str(), *prop, batch);
          }
          ++prop_count;
        } else {
          smsc_log_debug(logger, "property %s not found in profile %s", restore_properties[i], pf.getKey().c_str());               
        }
      }
      if (sendToPers) {
        pc.FinishPrepareBatch(prop_count, batch);
        pc.RunBatch(batch);
        smsc_log_debug(logger, "send %d properties to pers for profile %s", prop_count, pf.getKey().c_str());
      }
      return 1;
     } catch(const SerialBufferOutOfBounds &e) {
       smsc_log_warn(logger, "SerialBufferOutOfBounds Bad data in buffer read");
       return 0;
     }
  }

private:
  int blockSize;
  int blocksInFile;
  smsc::logger::Logger* logger;
  string				dbName;
  string				dbPath;
  PersClient& pc;

  // bukind: IT SEEMS THAT THIS CODE IS NEVER USED.
  // it did not have the following declaration!
  std::vector<File*>                    dataFile_f;

};

}//util
}//pers
}//scag

#endif
