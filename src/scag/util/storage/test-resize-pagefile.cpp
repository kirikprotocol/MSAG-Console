//
// Resizing pagefile
//

#include <cstdio>
#include <ctime>
#include "logger/Logger.h"
#include "scag/pvss/profile/Profile.h"
#include "PageFileDiskStorage2.h"
#include "DiskHashIndexStorage.h"
#include "IndexedStorage2.h"

using namespace scag2::util::storage;
using namespace scag2::pvss;

typedef PageFileDiskStorage2 DiskDataStorage;
typedef DiskHashIndexStorage< IntProfileKey, DiskDataStorage::index_type > DiskIndexStorage;
typedef IndexedStorage2< DiskIndexStorage, DiskDataStorage > DiskStorage;

void usage( const char* progname )
{
    std::fprintf(stderr,"USAGE: %s fullPathToStorageWoExt newPageSize\n",progname);
}


int main( int argc, char** argv )
{
    if ( argc < 3 ) {
        usage(argv[0]);
        return -1;
    }

    std::string fpath(argv[1]);
    unsigned pageSize;
    {
        char* endptr;
        pageSize = unsigned(strtoul(argv[2],&endptr,0));
        if ( *endptr != '\0') {
            std::fprintf(stderr,"wrong value '%s' for pageSize\n",argv[2]);
            usage(argv[0]);
            return -1;
        } else if ( pageSize < 256 ) {
            std::fprintf(stderr,"pageSize=%u should not be less than 256\n",pageSize);
            usage(argv[0]);
            return -1;
        }
    }
    
    // logger init
    smsc::logger::Logger::initForTest(smsc::logger::Logger::LEVEL_DEBUG);
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");

    smsc_log_info(logger,"new page size will be %u",pageSize);

    // opening the old storage
    const std::string fold(fpath);
    const std::string fnew(fpath + "-tmp");

    std::auto_ptr<DiskStorage> ostore;
    {
        smsc_log_info(logger,"opening old storage");
        std::auto_ptr< DiskDataStorage::storage_type > pf(new DiskDataStorage::storage_type );
        try {
            pf->Open(fold+".bin");
        } catch (...) {
            smsc_log_error(logger,"file %s.bin is not found",fold.c_str());
            usage(argv[0]);
            return -1;
        }

        if ( pf->getPageSize() >= pageSize ) {
            smsc_log_info(logger,"pageSize=%u is already >= than requested %u",
                          pf->getPageSize(),pageSize);
            return -1;
        }

        std::auto_ptr< DiskDataStorage > data( new DiskDataStorage(pf.release(),
                                                                   smsc::logger::Logger::getInstance("olddd")));
        smsc_log_debug(logger,"data storage is opened");
        std::auto_ptr< DiskIndexStorage > index( new DiskIndexStorage(fold+".idx",
                                                                      1000U,
                                                                      smsc::logger::Logger::getInstance("oldix")));
        smsc_log_debug(logger,"index storage is opened");
        ostore.reset(new DiskStorage(index.release(),data.release()));
    }

    // creating the new storage
    std::auto_ptr< DiskStorage > nstore;
    {
        try {
            smsc::core::buffers::File::Unlink((fnew+".bin").c_str());
        } catch (...) {
        }
        std::auto_ptr< DiskDataStorage::storage_type > pf(new DiskDataStorage::storage_type);
        try {
            pf->Create(fnew+".bin", pageSize, 1000);
        } catch (...) {
            smsc_log_error(logger,"file %s.bin cannot be created",fnew.c_str());
            usage(argv[0]);
            return -1;
        }

        std::auto_ptr< DiskDataStorage > data( new DiskDataStorage(pf.release(),
                                                                   smsc::logger::Logger::getInstance("newdd")));
        smsc_log_debug(logger,"data storage is created");

        try {
            smsc::core::buffers::File::Unlink((fnew+".idx").c_str());
        } catch (...) {
        }
        std::auto_ptr< DiskIndexStorage > index( new DiskIndexStorage(fnew+".idx",
                                                                      1000U,
                                                                      smsc::logger::Logger::getInstance("newix")));
        smsc_log_debug(logger,"index storage is created");
        nstore.reset(new DiskStorage(index.release(),data.release()));
    }

    // main loop
    DiskStorage::key_type    k;
    DiskStorage::buffer_type b;
    for ( DiskStorage::iterator_type i(ostore->begin()); i.next(k,b); ) {
        nstore->set(k,b);
        smsc_log_info(logger,"key=%s written",k.toString().c_str());
    }

    // closing everything
    nstore.reset(0);
    ostore.reset(0);

    // renaming
    smsc_log_info(logger,"renaming files...");
    char buf[40];
    {
        struct tm stm;
        stm.tm_isdst = 0;
        const time_t curt = time(0);
        localtime_r(&curt,&stm);
        std::strftime(buf,sizeof(buf),"-%Y%m%d%H%M%S",&stm);
    }
    smsc::core::buffers::File::Rename((fold+".bin").c_str(),(fold+".bin"+buf).c_str());
    smsc::core::buffers::File::Rename((fold+".idx").c_str(),(fold+".idx"+buf).c_str());
    smsc::core::buffers::File::Rename((fnew+".bin").c_str(),(fold+".bin").c_str());
    smsc::core::buffers::File::Rename((fnew+".idx").c_str(),(fold+".idx").c_str());
    return 0;
}
