#include "scag/util/storage/BlocksHSStorage2.h"
#include "scag/util/storage/DataFileManager.h"
// #include "scag/util/storage/Glossary.h"

//
// A test utility to collect storage statistics
//

int main( int argc, const char** argv )
{
    // typedef BHDiskStorage2< AbntAddr, Profile, BlocksHSStorage2 > DiskDataStorage;
    // typedef DiskDataStorage::storage_type storage_type;
    typedef scag2::util::storage::BlocksHSStorage2 storage_type;
    // typedef DiskDataStorage::value_type   value_type;

    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_INFO );
    smsc::logger::Logger* mainlog = smsc::logger::Logger::getInstance("main");

    if ( argc <= 1 ) {
        smsc_log_error(mainlog,"please specify fullpath on commanline");
        return -1;
    }

    scag2::util::storage::DataFileManager dfm(1,0,1000000);
    try {

        ++argv;
        for ( ; *argv != 0; ++argv ) {

            const std::string fullpath = *argv;
            const size_t slash = fullpath.rfind('/');
            std::string dbpath, dbname;
            if ( slash == std::string::npos ) {
                dbpath = ".";
                dbname = fullpath;
            } else {
                dbpath = fullpath.substr(0,slash);
                dbname = fullpath.substr(slash+1);
            }

            std::auto_ptr<storage_type> bs
                ( new storage_type
                  ( dfm,
                    smsc::logger::Logger::getInstance("pvssbh")));
        
            if ( bs->open(dbname,dbpath,true) != 0 ) {
                throw smsc::util::Exception("cannot open %s", fullpath.c_str());
            }

            const size_t blockSize = bs->blockSize();

            for ( storage_type::Iterator it(*bs.get()); it.next(); ) {
                storage_type::buffer_type& buf = it.getBuffer();
                const size_t ps = buf.size() - bs->idxSize();
                printf("pf %llu %u %u\n", it.getIndex(),
                       unsigned(ps), unsigned((ps+blockSize-1)/blockSize));
            }

        }

    } catch ( std::exception& e ) {
        fprintf(stderr,"exception: %s\n",e.what());
        std::terminate();
        return -1;
    }
    return 0;
}
