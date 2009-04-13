#include "scag/pvss/profile/AbntAddr.hpp"
#include "scag/pvss/profile/Profile.h"
#include "BlocksHSStorage3.h"
#include "Glossary.h"
#include "DataFileManager.h"

using namespace scag2::util::storage;
using namespace scag2::util;
using namespace scag2::pvss;

typedef  BlocksHSStorage3< AbntAddr, Profile >  store_type;
typedef  store_type::data_type                  data_type;

int main()
{
    Logger::Init();
    Logger* logger = Logger::getInstance("main");

    DataFileManager manager(1,500);
    Glossary* glossary = new Glossary();
    glossary->Open("storage/testglossary");
    
    store_type bhs(manager,glossary,Logger::getInstance("bhs"));
    int ret = bhs.Open( "testdata", "storage" );
    if ( ret < 0 ) {
        smsc_log_info(logger,"cannot open %d", ret);
        ret = bhs.Create( "testdata", "storage", 100, 1024 );
        smsc_log_info(logger,"created %d",ret);
    } else {
        smsc_log_info(logger,"opened %d", ret);
    }

    if ( ret < 0 ) {
        smsc_log_warn(logger, "cannot proceed" );
    }

    AbntAddr key( 11, 1, 1, "79137654079" );

    data_type data;
    data.value = new data_type::value_type;
    data.value->AddProperty( Property("bye","bye,life",INFINIT,time_t(-1),uint32_t(-1)) );
    data.backup = new data_type::backup_type;
    try {

        store_type::index_type oldBlockIndex;
        store_type::index_type blockIndex;
        if ( !bhs.Add(data,key,oldBlockIndex) || !bhs.Add(data,key,blockIndex) ) {
            smsc_log_warn(logger, "cannot add" );
        } else {
            smsc_log_info(logger, "added, index=%llx", blockIndex );
        }
        
        data.value->AddProperty( Property("hithere","hello,world",INFINIT,time_t(-1),uint32_t(-1)) );

        if ( !bhs.Change( data, key, blockIndex ) ) {
            smsc_log_warn(logger, "cannot change" );
        } else {
            smsc_log_info(logger, "changed, index=%llx", blockIndex );
        }

        bhs.Remove( key, blockIndex, data );

        if (!bhs.Get( oldBlockIndex, data )) {
            smsc_log_warn(logger, "cannot change" );
        } else {
            smsc_log_info(logger, "data got, size=%u", oldBlockIndex, unsigned(data.value->getProperties().GetCount()) );
        }

    } catch ( std::exception& e ) {
        smsc_log_error( logger, "exception: %s", e.what());
    }
    delete data.backup;
    delete data.value;
    return 0;
}
