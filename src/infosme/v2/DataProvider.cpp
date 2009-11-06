#include "DataProvider.h"
#include "db/DataSource.h"

namespace smsc {
namespace infosme2 {

using namespace smsc::util::config;

DataProvider::DataProvider() :
log_(smsc::logger::Logger::getInstance("is2.datapro"))
{
}


DataProvider::~DataProvider()
{
    char* key;
    smsc::db::DataSource* val;
    MutexGuard mg(lock_);
    for ( smsc::core::buffers::Hash< smsc::db::DataSource* >::Iterator i(&dss_);
          i.Next(key,val); ) {
        delete val;
    }
    dss_.Empty();
}


void DataProvider::init(ConfigView* config)
{
    // FIXME
}


smsc::db::DataSource* DataProvider::getDataSource( const char* dsid )
{
    MutexGuard mg(lock_);
    smsc::db::DataSource** ptr = dss_.GetPtr(dsid);
    if ( !ptr ) return 0;
    return *ptr;
}

}
}
