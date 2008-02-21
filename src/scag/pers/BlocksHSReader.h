#ifndef ___BLOCKS_HS_READER_H
#define ___BLOCKS_HS_READER_H

#include "BlocksHSStorage.h"
#include "Profile.h"

namespace scag { namespace pers {

const int BLOCK_SIZE = 2048;
const int BLOCKS_IN_FILE = 50000;

template<class Key>
class BlocksHSReader
{
public:
	typedef templDataBlockHeader<Key> DataBlockHeader;

    BlocksHSReader(const string& _dbName, const string& _dbPath, int _blockSize = BLOCK_SIZE,
                    int _blocksInFile = BLOCKS_IN_FILE): dbName(_dbName), dbPath(_dbPath),
                     blockSize(_blockSize), blocksInFile(_blocksInFile) { 
      logger = smsc::logger::Logger::getInstance("pers_up");
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
    
    int readDataFiles(int files_count) {
      if (!files_count) {
        return 0;
      }

      char	buff[10];
      string path = dbPath + '/' + dbName +'/' + dbName + "-data";
      long effectiveBlockSize = blockSize - sizeof(DataBlockHeader);
      char* data_buff;
      int total_count = 0;
      smsc_log_debug(logger, "start read %d files", files_count);

      for(long i = 0; i < files_count; i++)
      {
          snprintf(buff, 10, "-%.7d", i);
          string name = path + buff;
          try {
            int profiles_count = 0;
            smsc_log_debug(logger, "Open data file %d: %s", i + 1, name.c_str());
            File dataFile;
            dataFile.ROpen(name.c_str());
            dataFile.SetUnbuffered();
            for (int j = 0; j < blocksInFile; ++j) {
              //dataFile.Seek(i * blockSize, SEEK_SET);
              DataBlockHeader hdr;
              dataFile.Read((void*)&hdr, sizeof(DataBlockHeader));
              if (hdr.block_used != BLOCK_USED) {
                smsc_log_warn(logger, "data block number=%d is not used", j);
                dataFile.Seek(effectiveBlockSize);
                continue;
              }
              if (!hdr.head) {
                smsc_log_error(logger, "long data block number=%d pfkey=%s", j, hdr.key.toString().c_str());
                dataFile.Seek(effectiveBlockSize);
                continue;
              }
              ++profiles_count;
              smsc_log_debug(logger, "profile key=%s", hdr.key.toString().c_str());
              smsc_log_debug(logger, "header data size=%d", hdr.data_size);

              SerialBuffer data;
              //data.setBuffLength(hdr.data_size);
              data.setBuffLength(effectiveBlockSize);
              data_buff = data.ptr();
              data.setLength(effectiveBlockSize);
              //data.setLength(hdr.data_size);
              dataFile.Read((void*)data_buff, effectiveBlockSize);
              //dataFile.Read((void*)data_buff, hdr.data_size);

              /*
              Profile pf(hdr.key.toString());   
              pf.Deserialize(data, true);

              char *prop_key = 0;
              Property *prop;
              PropertyHash::Iterator it = pf.getProperties().getIterator();

              while(it.Next(prop_key, prop)) {
                if (prop_key && prop) {
                  smsc_log_debug(logger, "property name='%s' property:'%s'", prop_key, prop->toString().c_str());
                }
              }
              */
            }
            total_count += profiles_count;
            smsc_log_debug(logger, "profiles count = %d/%d in file %d", profiles_count, total_count, i + 1);
            dataFile.Close();
          } catch (const FileException& ex) {
            smsc_log_error(logger, "Cannot open data file: %s", ex.what());
            return CANNOT_OPEN_DATA_FILE;
          }
      }
      smsc_log_debug(logger, "total profiles count = %d in %d files", total_count, files_count);
      return 0;

    }
private:
  int blockSize;
  int blocksInFile;
  smsc::logger::Logger* logger;
  string				dbName;
  string				dbPath;

};

}//pers
}//scag

#endif
