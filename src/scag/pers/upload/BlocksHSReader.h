#ifndef ___BLOCKS_HS_READER_H
#define ___BLOCKS_HS_READER_H

#include "scag/pers/BlocksHSStorage.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pers/Profile.h"
#include "scag/pers/upload/PersClient.h"
#include "core/synchronization/EventMonitor.hpp"

namespace scag { namespace pers { namespace util {

  using scag::util::storage::SerialBuffer;
  using scag::util::storage::SerialBufferOutOfBounds;
  using namespace scag::pers;
  using scag::pers::util::PersClient;
  using scag::pers::util::PersClientException;
  using smsc::core::synchronization::EventMonitor;
  using smsc::core::synchronization::MutexGuard;
  //using scag::pers::client::PersClient;
  //using scag::pers::client::PersClientException;
  
  static const int MAX_RESEND_COUNT = 10;

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

  static const char* minsk_properties[] = {
    "sr_BE.exclude",
    "sr_BE.sr_UniBalance.exclude",
    "sr_BE.sr_callback.exclude",
    "sr_BE.sr_MCA.exclude",
    "sr_BE.sr_10.exclude",
    "sr_BE.sr_test.exclude"
  };
  const int MINSK_PROPERTIES_COUNT = 6;

  static const char* cmbpmb_properties[] = {
    "cmb.subscription",
    "cmb.usages",
    "pmb.subscription",
    "pmb.usages"
  };

  const int CMBPMB_PROPERTIES_COUNT = 4;
  const int MAX_BATCH_SIZE = 500;

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
      cmbpmbLogger = smsc::logger::Logger::getInstance("cmbpmb");
      propLogger = smsc::logger::Logger::getInstance("property");
      smsc_log_debug(logger, "dbName='%s' dbPath='%s' blockSize=%d blocksInFile=%d",
                     dbName.c_str(), dbPath.c_str(), blockSize, blocksInFile);
      addresses.push_back(Key(".1.1.5434990229"));
      addresses.push_back(Key(".1.1.21000000022"));
      addresses.push_back(Key(".1.1.21000000823"));
      addresses.push_back(Key(".1.1.21000001142"));
      addresses.push_back(Key(".1.1.21000001148"));
      addresses.push_back(Key(".1.1.21000001149"));
      addresses.push_back(Key(".1.1.21000001230"));
      addresses.push_back(Key(".1.1.21000001231"));
      addresses.push_back(Key(".1.1.254"));

      //straddresses.push_back(".1.1.5434990229");
      straddresses.push_back(".1.1.21000000022");
      straddresses.push_back(".1.1.21000000823");
      //straddresses.push_back(".1.1.21000001142");
      //straddresses.push_back(".1.1.21000001148");
      //straddresses.push_back(".1.1.21000001149");
      //straddresses.push_back(".1.1.21000001230");
      //straddresses.push_back(".1.1.21000001231");
      straddresses.push_back(".1.1.254");
    }

    ~BlocksHSReader() {
    }

    bool needProfileKey(const Key& key) {
        std::vector<Key>::iterator result = addresses.end();
        result = std::find(addresses.begin(), addresses.end(), key);
        return result == addresses.end() ? false : true;
    }

    bool needProfileKey(const string& key) {
        std::vector<string>::iterator result = straddresses.end();
        result = std::find(straddresses.begin(), straddresses.end(), key);
        return result == straddresses.end() ? false : true;
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

    bool getProfileData(int curFileNumber, File* dataFile, DataBlockHeader &hdr, SerialBuffer& data) {
      try {
        char* dataBuff;
        data.setBuffLength(hdr.data_size);
        dataBuff = data.ptr();
        data.setLength(hdr.data_size);
        dataFile->Read((void*)dataBuff, hdr.data_size);
        if (hdr.total_blocks == 1) {
          return true;
        }
        smsc_log_info(logger, "read long profile: pfkey=%s blocks:%d data size:%d",
                               hdr.key.toString().c_str(), hdr.total_blocks, hdr.data_size);
        long curBlockIndex = hdr.next_block;
        long effectiveBlockSize = blockSize - sizeof(DataBlockHeader);
        int i = 1;
        File *f;
        do {
          std::auto_ptr<File> filePtr;
          int file_number = curBlockIndex / blocksInFile;
          if (file_number == curFileNumber) {
            f = dataFile;
          } else {
            f = new File();
            filePtr.reset(f);
            f->ROpen(getFileName(file_number).c_str());
          }

          off_t offset = (curBlockIndex - file_number * blocksInFile) * blockSize;
          f->Seek(offset, SEEK_SET);
          DataBlockHeader curhdr;
          f->Read((void*)&hdr, sizeof(DataBlockHeader));
          if(hdr.block_used != BLOCK_USED) {
              smsc_log_error(logger, "read long profile: block index=%d unused", curBlockIndex);
              return false;
          }
          curBlockIndex = hdr.next_block;
          size_t dataSize = curBlockIndex == -1 ? hdr.data_size - effectiveBlockSize*i : effectiveBlockSize;
          f->Read((void*)(dataBuff+(i*effectiveBlockSize)), dataSize);
          ++i;
        } while (curBlockIndex != -1);
        return true;
      } catch (const FileException& ex) {
        smsc_log_error(logger, "error reading profile: %s", ex.what());
        return false;
      }
    }

    string getFileName(int fileNumber) {
      char	postfix[10];
      string name = dbPath + '/' + dbName +'/' + dbName + "-data";
      snprintf(postfix, 10, "-%.7d", fileNumber);
      return name + postfix;
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
      int matched_total = 0;
      smsc_log_debug(logger, "start read %d files", files_count);
      std::vector<string> cmbpmbNames;
      for (int i = 0; i < CMBPMB_PROPERTIES_COUNT; ++i) {
        cmbpmbNames.push_back(cmbpmb_properties[i]);
      }

      for(long i = 0; i < files_count; i++)
      {
          snprintf(buff, 10, "-%.7d", i);
          string name = path + buff;
          try {
            int profiles_count = 0;
            int status_profiles = 0;
            int matched_infile = 0;
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
                smsc_log_info(logger, "long data block number=%d pfkey=%s blocks:%d data size:%d",
                                j, hdr.key.toString().c_str(), hdr.total_blocks, hdr.data_size);
                continue;
              }
              if (!needProfileKey(hdr.key.toString())) {
                  continue;
              }
              smsc_log_info(logger, "profile key=%s", hdr.key.toString().c_str());
              SerialBuffer data;
              if (!getProfileData(i, &dataFile, hdr, data)) {
                smsc_log_error(logger, "error reding profile block number=%d pfkey=%s blocks:%d data size:%d",
                               j, hdr.key.toString().c_str(), hdr.total_blocks, hdr.data_size);
                continue;
              }
              ++profiles_count;

              //status_profiles += restoreProfile(hdr.key, data, sendToPers);
              
              int matched = 0;
              int restoreResult = restoreProfileCompletely(hdr.key, data, sendToPers, cmbpmbNames, matched);
              //int restoreResult = restoreProfileMinsk(hdr.key, data, sendToPers, matched);
              if (sendToPers && restoreResult == -1) {
                int resendCount = 0;
                while (restoreResult == -1 && resendCount < MAX_RESEND_COUNT) {
                  smsc_log_warn(logger, "resending profile key='%s', try number %d",
                                        hdr.key.toString().c_str(), resendCount + 1);
                  int rematched = 0;
                  restoreResult = restoreProfileCompletely(hdr.key, data, sendToPers, cmbpmbNames, matched);
                  //restoreResult = restoreProfileMinsk(hdr.key, data, sendToPers, rematched);
                  ++resendCount;
                }
                if (restoreResult == -1) {
                  smsc_log_warn(logger, "Can't upload profile key='%s'", hdr.key.toString().c_str());
                  continue;
                }              
                if (restoreResult == 1) {
                  smsc_log_info(logger, "profile key='%s' uploaded", hdr.key.toString().c_str());
                }              
              }
              status_profiles += restoreResult;
              matched_infile += matched;
            }
            matched_total += matched_infile;
            total_count += profiles_count;
            total_status_profiles += status_profiles;
            smsc_log_info(logger, "read profiles count     = %d/%d   in file %d/total", profiles_count, total_count, i + 1);
            smsc_log_info(logger, "matched profiles count  = %d/%d   in file %d/total", matched_infile, matched_total, i + 1);
            smsc_log_info(logger, "uploaded profiles count = %d/%d   in file %d/total", status_profiles, total_status_profiles, i + 1);
            dataFile.Close();
          } catch (const FileException& ex) {
            smsc_log_error(logger, "Cannot open data file: %s", ex.what());
            return CANNOT_OPEN_DATA_FILE;
          }
      }
      smsc_log_info(logger, "total read profiles count = %d in %d files", total_count, files_count);
      smsc_log_info(logger, "total matched profiles    = %d", matched_total);
      smsc_log_info(logger, "total uploaded profiles   = %d", total_status_profiles);
      return 0;
    }
private:
  int restoreProfileCompletely(const Key& key, SerialBuffer& data, bool sendToPers, const std::vector<std::string>& matchNames, int& matched) {
    try {
      Profile pf(key.toString(), logger);  
      data.SetPos(0);
      pf.Deserialize(data, true);
      
      //if (!pf.GetProperty("srv_BE.advTags")) {
        //return 0;
      //}

      int batchCount = pf.getProperties().GetCount() / MAX_BATCH_SIZE;
      batchCount += pf.getProperties().GetCount() % MAX_BATCH_SIZE != 0 ? 1 : 0;
      smsc_log_info(logger, "active properties count: %d", pf.getProperties().GetCount());
      int allPropCount = 0;
      PropertyHash::Iterator it = pf.getProperties().getIterator();
      for (int i = 0; i < batchCount; ++i) {
          SerialBuffer batch;
          if (sendToPers) {
            pc.PrepareBatch(batch);
          }
    
          Property* prop;
          char *propkey = 0;
          int prop_count = 0;
          while(prop_count < MAX_BATCH_SIZE && it.Next(propkey, prop)) {
            //std::vector<string>::const_iterator it = std::find(matchNames.begin(), matchNames.end(), string(prop->getName()));
            //if (it != matchNames.end()) {
              //smsc_log_debug(cmbpmbLogger, "key=%s property=%s", pf.getKey().c_str(), prop->toString().c_str());
              //continue;
            //}
            if (prop->getType() < PropertyType::INT || prop->getType() > PropertyType::DATE) {
              smsc_log_warn(logger, "unknown property type: %d, convert to INT. key=%s property=%s", prop->getType(), pf.getKey().c_str(), propkey);
              prop->setIntValue(0);
            }
            if (prop->getTimePolicy() <= UNKNOWN || prop->getTimePolicy() > W_ACCESS) {
              smsc_log_warn(logger, "unknown time policy: %d, convert to INFINIT. key=%s property=%s", prop->getTimePolicy(), pf.getKey().c_str(), propkey);
              prop->setTimePolicy(INFINIT, 0, 0);
            }
            if (sendToPers) {
              pc.SetPropertyPrepare(PT_ABONENT, key.toString().c_str(), *prop, batch);
            }
            smsc_log_debug(propLogger, "key=%s property=%s", pf.getKey().c_str(), prop->toString().c_str());
            ++prop_count;
          }
          if (sendToPers && prop_count > 0) {
            pc.FinishPrepareBatch(prop_count, batch);
            pc.RunBatch(batch);
            smsc_log_debug(logger, "send %d properties to pers for profile key=%s", prop_count, pf.getKey().c_str());

            for (int i = 0; i < prop_count; ++i) {
              pc.SetPropertyResult(batch);
            }
            //return 1;
          }
          allPropCount += prop_count;
      }
      matched = allPropCount > 0 ? 1 : 0;
      smsc_log_info(logger, "profile key=%s matched properties count=%d", pf.getKey().c_str(), allPropCount);
      if (sendToPers && matched) {
          return 1;
      }
    } catch (const SerialBufferOutOfBounds &e) {
      smsc_log_warn(logger, "Error reading profile key=%s. SerialBufferOutOfBounds: bad data in buffer read ", key.toString().c_str());
    } catch (const PersClientException& ex) {
      smsc_log_warn(logger, "Error uploading profile key=%s. PersClientException: %s", key.toString().c_str(), ex.what());
      return -1;
    }
    return 0;
  }

  int restoreProfileMinsk(const Key& key, SerialBuffer& data, bool sendToPers, int& matched) {
    try {
      Profile pf(key.toString());   
      pf.Deserialize(data, true);
      int prop_count = 0;
      SerialBuffer batch;
      if (sendToPers) {
        pc.PrepareBatch(batch);
      }
      Property* prop = 0;
      for (int i = 0; i < CMBPMB_PROPERTIES_COUNT; ++i) {
        //for (int i = 0; i < MINSK_PROPERTIES_COUNT; ++i) {
        //if (prop = pf.GetProperty(minsk_properties[i])) {
        if (prop = pf.GetProperty(cmbpmb_properties[i])) {
          smsc_log_debug(logger, "property '%s' found in profile %s", cmbpmb_properties[i], pf.getKey().c_str());
          smsc_log_debug(cmbpmbLogger, "key=%s property=%s", pf.getKey().c_str(), prop->toString().c_str());
          //smsc_log_debug(logger, "property '%s' found in profile %s", minsk_properties[i], pf.getKey().c_str());
          if (sendToPers) {
            pc.SetPropertyPrepare(PT_ABONENT, key.toString().c_str(), *prop, batch);
          }
          ++prop_count;
        } else {
          smsc_log_debug(logger, "property '%s' not found in profile %s", cmbpmb_properties[i], pf.getKey().c_str());
          //smsc_log_debug(logger, "property '%s' not found in profile %s", minsk_properties[i], pf.getKey().c_str());
        }
      }
      matched = prop_count > 0 ? 1 : 0;
      if (sendToPers && prop_count > 0) {
        pc.FinishPrepareBatch(prop_count, batch);
        pc.RunBatch(batch);
        smsc_log_debug(logger, "send %d properties to pers for profile %s", prop_count, pf.getKey().c_str());
        //return 1;
      }
      if (sendToPers && prop_count > 0) {
        for (int i = 0; i < prop_count; ++i) {
          pc.SetPropertyResult(batch);
        }
        return 1;
      }
     } catch(const SerialBufferOutOfBounds &e) {
       smsc_log_warn(logger, "Error reading profile key=%s. SerialBufferOutOfBounds Bad data in buffer read", key.toString().c_str());
     } catch (const PersClientException& ex) {
       smsc_log_warn(logger, "Error uploading profile key=%s. PersClientException: %s", key.toString().c_str(), ex.what());
       return -1;
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
        smsc_og_debug(logger, "property %s not found in profile %s", INC_PROPERTY_NAME, pf.getKey().c_str());               
      }*/
      for (int i = 0; i < RESTORE_PROPERTIES_COUNT; ++i) {
        if (prop = pf.GetProperty(restore_properties[i])) {
          smsc_log_debug(logger, "property '%s' found in profile %s", restore_properties[i], pf.getKey().c_str());
          if (sendToPers) {
            pc.SetPropertyPrepare(PT_ABONENT, key.toString().c_str(), *prop, batch);
          }
          ++prop_count;
        } else {
          smsc_log_debug(logger, "property '%s' not found in profile %s", restore_properties[i], pf.getKey().c_str());
        }
      }
      if (sendToPers) {
        pc.FinishPrepareBatch(prop_count, batch);
        pc.RunBatch(batch);
        smsc_log_debug(logger, "send %d properties to pers for profile %s", prop_count, pf.getKey().c_str());
        return 1;
      }
     } catch(const SerialBufferOutOfBounds &e) {
       smsc_log_warn(logger, "Error reading profile key=%s. SerialBufferOutOfBounds Bad data in buffer read", key.toString().c_str());
     } catch (const PersClientException& ex) {
       smsc_log_warn(logger, "Error uploading profile key=%s. PersClientException: %s", key.toString().c_str(), ex.what());
     }
     return 0;
  }

private:
  int blockSize;
  int blocksInFile;
  smsc::logger::Logger* logger;
  smsc::logger::Logger* cmbpmbLogger;
  smsc::logger::Logger* propLogger;
  string		dbName;
  string		dbPath;
  PersClient& pc;

  // bukind: IT SEEMS THAT THIS CODE IS NEVER USED.
  // it did not have the following declaration!
  std::vector<File*>                    dataFile_f;
  std::vector<Key> addresses;
  std::vector<string> straddresses;
  
  EventMonitor monitor;

};

}//util
}//pers
}//scag

#endif
