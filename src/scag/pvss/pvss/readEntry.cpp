//
// Test utility to read an entry in abonent datafile
//

#include <memory>
#include <iostream>
#include "scag/util/storage/Glossary.h"
#include "scag/util/storage/BHDiskStorage2.h"
#include "scag/util/storage/BlocksHSStorage2.h"
#include "scag/pvss/profile/AbntAddr.hpp"
#include "scag/pvss/profile/Profile.h"
#include "logger/Logger.h"

using namespace scag2::util::storage;
using namespace scag2::pvss;

int main( int argc, const char** argv )
{
    typedef BHDiskStorage2< AbntAddr, Profile, BlocksHSStorage2 > DiskDataStorage;
    typedef DiskDataStorage::storage_type storage_type;
    typedef DiskDataStorage::value_type   value_type;

    smsc::logger::Logger::Init();
    const char** argp = argv;
    ++argp;

    std::string fullpath;
    DataFileManager dfm(1,0,10000000);
    std::auto_ptr<DiskDataStorage> dds;
    try {

        if ( *argp != 0 ) {
            fullpath = *argp;
            printf("path: %s\n", fullpath.c_str() );
        } else {
            throw smsc::util::Exception("please specify fullpath on commandline");
        }

        const size_t slash = fullpath.rfind('/');
        std::string dbpath, dbname;
        if ( slash == std::string::npos ) {
            dbpath = ".";
            dbname = fullpath;
        } else {
            dbpath = fullpath.substr(0,slash);
            dbname = fullpath.substr(slash+1);
        }

        std::auto_ptr< Glossary > glossary(new Glossary);
        const std::string glossfn = dbpath + "/glossary";
        if ( 0 != glossary->Open( glossfn ) ) {
            throw smsc::util::Exception("cannot open glossary %s", glossfn.c_str() );
        }

        std::auto_ptr<storage_type> bs
            ( new storage_type
              ( dfm,
                smsc::logger::Logger::getInstance("pvssbh")));

        if ( bs->open(dbname,dbpath,true) != 0 ) {
            throw smsc::util::Exception("cannot open %s", fullpath.c_str());
        }

        dds.reset( new DiskDataStorage
                   ( bs.release(),
                     glossary.release(),
                     smsc::logger::Logger::getInstance("pvssdd")) );

    } catch ( std::exception& e ) {
        fprintf( stderr, "exception: %s\n", e.what());
        exit(-1);
    }

    while ( ! std::cin.eof() ) {
        std::string pos;
        std::getline(std::cin,pos);
        if ( pos.empty() ) continue;
        char* endptr;
        const char* cstr = pos.c_str();
        storage_type::index_type idx = strtoul(cstr,&endptr,0);
        if ( *endptr != '\0' ) {
            fprintf( stderr, "entered: <%s>, should be index\n", cstr );
            continue;
        }
        printf("entered idx: %u/%x\n", unsigned(idx), unsigned(idx) );
        value_type val;
        val.value = new value_type::value_type;
        DataBlockBackupTypeJuggling2< value_type::value_type > dbbtj;
        try {

            if ( ! dds->read(idx) ) {
                throw smsc::util::Exception("cannot read at %x", unsigned(idx));
            }
            dds->deserialize( dbbtj.store2ref(val) );

        } catch ( std::exception& e ) {
            fprintf(stderr,"exception: %s\n", e.what() );
        }
        printf( "profile: %s\n", val.value->toString().c_str() );
        dbbtj.dealloc(val);
    }
    return 0;
}
