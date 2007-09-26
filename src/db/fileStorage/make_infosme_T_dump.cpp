#include <sys/types.h>
#include <time.h>

#include <db/fileStorage/ApplicationStatements.hpp>
#include <db/fileStorage/InfoSme_T_DBEntityStorage.hpp>
#include <db/fileStorage/DiskDataStorage.hpp>
#include <db/fileStorage/InfoSme_T_Entity_Adapter.hpp>
#include <db/fileStorage/InfoSme_T_Entity.hpp>

#include <logger/Logger.h>

int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");

  if ( argc != 2 ) {
    std::cerr << "Usage: " << argv[0] << "INFOSME_T_db_file_name" << std::endl;
    return 1;
  }
  const char* dbFileName = argv[1];

  try {
    SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* fileStorage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(dbFileName);
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      st =fileStorage->open();
    if ( st != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      //smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage:: can't create storage");
      return 1;
    }

    InfoSme_T_DBEntityStorage* infoSme_T_Storage = new InfoSme_T_DBEntityStorage(fileStorage, false);

    DbIterator<InfoSme_T_Entity>* iterator = infoSme_T_Storage->getIterator();
    InfoSme_T_Entity resultValue;
    while(iterator->nextValue(&resultValue)) 
      std::cout << resultValue.toString() << std::endl;
  } catch (std::exception& ex) {
    std::cerr << "Catched exception [" << ex.what() << "]. Terminated" << std::endl;
    return 1;
  }
  return 0;
}
