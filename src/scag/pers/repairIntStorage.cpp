#include <memory>
#include "logger/Logger.h"
#include "core/buffers/PageFile.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash64.hpp"
#include "Glossary.h"
#include "Profile.h"
#include "Property.h"
#include "VarRecSizeStore.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/util/io/HexDump.h"

using namespace scag2::util::storage;
using namespace scag2::util;
using namespace scag::pers;

void extractPageFile( smsc::logger::Logger* logger, PageFile& pf,
                      bool hasGlossary,
                      IntHash64< uint32_t >& indexHash,
                      VarRecSizeStore< IntProfileKey >* newStore )
{
    File::offset_type pageIndex = 0;
    std::vector< unsigned char > data;
    File::offset_type realIndex;
    try {
        while ( true ) {

            File::offset_type nextIndex;
            bool corrupted = false;
            try {

                smsc_log_info(logger,"=====================================================");
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

            Profile prof(logger);
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
                    smsc_log_info(logger,"profile key: %s",prof.getKey().c_str());
                }
            }

            std::string dump;
            {
                HexDump hd;
                unsigned dumpsize = std::min(unsigned(data.size()),unsigned(256));
                hd.hexdump(dump,(const char*)&data[0],dumpsize);
                dump += ": ";
                hd.strdump(dump,(const char*)&data[0],dumpsize);
            }

            smsc_log_info(logger,"page=%lld real=%lld datasize=%ld key=%d data=%s",
                          static_cast<long long>(pageIndex),
                          static_cast<long long>(realIndex),
                          long(data.size()),
                          pkey,
                          dump.c_str() );
            pageIndex = nextIndex;

            SerialBuffer sb(data.size());
            sb.Append((char*)&data[0],data.size());
            sb.SetPos(0);
            try {
                prof.Deserialize( sb, hasGlossary );
            } catch ( std::exception& e ) {
                smsc_log_warn(logger,"exception: %s", e.what());
            }

            const PropertyHash& phash = prof.getProperties();
            {
                PropertyHash::Iterator iter(&phash);
                char* key;
                Property* value;
                while ( iter.Next(key,value) ) {
                    smsc_log_info(logger," prop: %s", value->toString().c_str());
                }
            }
                
            if ( newStore && hasProfileKey ) {
                smsc_log_info(logger,"inserting profile %s to new store", profileKey.toString().c_str() );
                newStore->newRecord( profileKey, &prof );
            }

        }
    } catch ( std::exception& e ) {
        smsc_log_error(logger,"exception: %s", e.what());
    }
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
    smsc_log_info(logger,"=== extraction of disk hash data ===");
    DiskHash< IntProfileKey, OffsetValue >::Iterator iter(dh);
    IntProfileKey key;
    OffsetValue value;
    while ( iter.Next(key,value) ) {
        indexHash.Insert(value.value,*key.getKey());
        smsc_log_info(logger,"key=%s value=%ld", key.toString().c_str(), long(value.value) );
    }
}


void usage( const char* prog )
{
    fprintf(stderr,"Usage: %s [--create] int-storage-stem\n");
    fprintf(stderr,"  Example of int-storage-stem: /home/user/msag/storage/service (i.e. w/o extension)\n");
}


int main( int argc, const char** argv )
{
    bool createNew = false;
    std::string fn;
    for ( int i = 1; i < argc; ++i ) {
        std::string arg = argv[i];
        if ( arg == "--create" ) {
            createNew = true;
        } else {
            fn = arg;
        }
    }

    if ( fn.size() <= 0 ) {
        fprintf(stderr,"too short name\n");
        usage( argv[0]);
        exit(-1);
    }
    if ( fn[fn.size()-1] == '/' ) {
        fprintf(stderr,"filename should not ends with /\n");
        usage( argv[0]);
        exit(-1);
    }

    // getting a dirname
    std::string dirname;
    std::string basename;
    {
        size_t lastslash = fn.find_last_of('/');
        if ( lastslash == std::string::npos ) {
            dirname = ".";
            basename = fn;
        } else {
            dirname = fn.substr(0,lastslash);
            basename = fn.substr(lastslash+1);
        }
    }

    const std::string diskHashFilename = fn + ".idx";
    if ( ! File::Exists(diskHashFilename.c_str()) ) {
        fprintf(stderr,"disk hash does not exist at %s", diskHashFilename.c_str());
        usage( argv[0]);
        exit(-1);
    }

    const std::string pageFilename = fn + ".bin";
    if ( ! File::Exists(pageFilename.c_str()) ) {
        fprintf(stderr,"page file does not exist at %s", pageFilename.c_str());
        usage( argv[0]);
        exit(-1);
    }

    const std::string newStorageName = "./new" + basename;
    if ( createNew ) {
        fprintf(stderr,"a new storage will be created at %s\n", newStorageName.c_str());
    }

    smsc::logger::Logger::Init();
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");
    smsc_log_info(logger,"filename: %s, dirname: %s, basename: %s", fn.c_str(), dirname.c_str(), basename.c_str());

    try {

        bool hasGlossary;
        {
            std::string glossfn = dirname + "/glossary";
            hasGlossary = File::Exists(glossfn.c_str());
            if ( hasGlossary ) {
                smsc_log_info(logger,"opening glossary %s", glossfn.c_str());
                Glossary::Open(glossfn);
            } else {
                smsc_log_warn(logger,"glossary %s not found", glossfn.c_str());
            }
        }

        IntHash64< uint32_t > indexHash;
        {
            DiskHash< IntProfileKey, OffsetValue > dh;
            smsc_log_info(logger,"opening diskhash file %s",diskHashFilename.c_str());
            dh.Open( diskHashFilename.c_str(), true, false );
            extractDiskHash( logger, dh, indexHash );
            smsc_log_info(logger,"extracted %ld indices", long(indexHash.Count()));
        }

        std::auto_ptr<VarRecSizeStore< IntProfileKey > > newStore;
        if ( createNew ) {
            newStore.reset( new VarRecSizeStore< IntProfileKey > );
            try {
                File::Unlink( (newStorageName + ".bin").c_str() );
            } catch (...) {}
            try {
                File::Unlink( (newStorageName + ".idx").c_str() );
            } catch (...) {}
            newStore->init( newStorageName, 100 );
        }

        {
            PageFile pf;
            smsc_log_info(logger,"opening page file %s",pageFilename.c_str());
            pf.Open(pageFilename);
            extractPageFile( logger, pf, hasGlossary, indexHash, newStore.get() );
        }
        
    } catch ( std::exception& e ) {
        smsc_log_error(logger, "top level exception: %s", e.what());
        fprintf(stderr,"top level exception: %s\n", e.what());
    } catch (...) {
        smsc_log_error(logger, "unknown top level exception");
        fprintf(stderr,"unknown top level exception\n");
    }
    return 0;
}
