static const char ident[] = "$Id$";

#include <assert.h>
#include "inman/incache.hpp"

#define DIRECTORY_SEPARATOR '/'
                            
namespace smsc {
namespace inman {
namespace cache {
/* ************************************************************************** *
 * class AbonentCache implementation:
 * ************************************************************************** */
// Hash<>:Node  +  AccessList:Node
//sizeof(AbonentId)*2 + sizeof(AbonentRecordRAM) + sizeof(AbonentId)*2 

static const unsigned int DFLT_RAM = 40; //Mb
static const char _nmCch[] = "icache7.dat";

AbonentCache::AbonentCache(AbonentCacheCFG * cfg, Logger * uselog/* = NULL*/)
    : _cfg(*cfg), useFile(false)
{
    static const unsigned int DFLT_FACTOR = sizeof(AbonentRecordRAM) + sizeof(AbonentId)*4;
    assert(_cfg.nmDir);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.InCache");
    maxRamIt = (((_cfg.RAM ? _cfg.RAM : DFLT_RAM)<<10)/DFLT_FACTOR)<<10;
    smsc_log_info(logger, "InCache: RAM cache: %u abonents.", maxRamIt);

    std::string nmFile(_cfg.nmDir);
    if (nmFile[nmFile.length()] != DIRECTORY_SEPARATOR)
        nmFile += DIRECTORY_SEPARATOR;
    nmFile += _nmCch;
    load(nmFile.c_str());
    return;
}

AbonentCache::~AbonentCache()
{
    cache.Empty();
    close();
}

static const int DFLT_CACHE_ELEM = 1; //500
bool AbonentCache::load(const char * file_nm)
{
    try {
        bool present = File::Exists(file_nm);
        if (present)
            flCache.Open(file_nm, false, false);
        else
            flCache.Create(file_nm, DFLT_CACHE_ELEM, false);
        useFile = true;
        smsc_log_info(logger, "InCache: %s file cache %s.",
                      present ? "reading" : "creating", file_nm);
    } catch (std::exception & exc) {
        smsc_log_error(logger, "InCache: %s", exc.what());
        smsc_log_info(logger, "InCache: proceeding in RAM only mode.");
    }
    return useFile; 
}

//flCache.GetFile().getFileName()
bool AbonentCache::close(void)
{
    try {
        if (useFile) {
            flCache.Close();
            useFile = false;
        }
    } catch (std::exception & exc) {
        smsc_log_error(logger, "InCache: %s", exc.what());
    }
    return !useFile;
}

//Returns: 0 - update, 1 - addition
int AbonentCache::ramInsert(const AbonentId & ab_number, AbonentBillType ab_type,
                                                                time_t & queried)
{
    int status = 0; 
    if (!queried)
        queried = time(NULL);

    AbonentRecordRAM * pabRec = cache.GetPtr(ab_number.getSignals());
    if (!pabRec) {
        AbonentRecordRAM ab_rec(ab_type, queried);
        status = cache.Insert(ab_number.getSignals(), ab_rec);
        pabRec = cache.GetPtr(ab_number.getSignals());
        if (accList.size() >= maxRamIt)
            accList.pop_back();
    } else { //update
        pabRec->ab_type = ab_type;
        pabRec->tm_queried = queried;
        accList.erase(pabRec->accIt);
    }
    accList.push_front(ab_number);
    pabRec->accIt = accList.begin();
    return status;
}

AbonentBillType AbonentCache::ramLookUp(AbonentId & ab_number)
{
    AbonentRecordRAM * pabRec = cache.GetPtr(ab_number.getSignals());
    if (pabRec) {
        if (pabRec->accIt != accList.begin()) {
            accList.erase(pabRec->accIt);
            accList.push_front(ab_number);
            pabRec->accIt = accList.begin();
        }
        if (pabRec->ab_type != btUnknown) {
            if (time(NULL) < (pabRec->tm_queried + _cfg.interval))
                return pabRec->ab_type;
        }
        pabRec->ab_type = btUnknown; //expired
        smsc_log_debug(logger, "InCache: abonent %s info is expired",
                       ab_number.getSignals());
    }
    return btUnknown;
}

// ----------------------------------------
// AbonentCacheITF interface methods:
// ----------------------------------------
void AbonentCache::setAbonentInfo(const AbonentId & ab_number, AbonentBillType ab_type,
                                    time_t queried /*= 0*/)
{
    MutexGuard grd(cacheGuard);
    int status = ramInsert(ab_number, ab_type, queried); //sets time

    if (ab_type != btUnknown) {
        try {
            flCache.Insert(AbonentHashKey(ab_number),
                            AbonentHashData(ab_type, queried), true);
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: abonent %s: %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    smsc_log_debug(logger, "InCache: abonent %s is %s: %u",
                   ab_number.getSignals(), status ? "added" : "updated", ab_type);
}

AbonentBillType AbonentCache::getAbonentInfo(AbonentId & ab_number)
{
    MutexGuard  guard(cacheGuard);
    AbonentBillType ab_type = ramLookUp(ab_number);
    if (ab_type == btUnknown) { //look in file cache
        AbonentHashData     ab_rec;
        try {
            if (flCache.LookUp(AbonentHashKey(ab_number), ab_rec)) {
                ramInsert(ab_number, ab_rec.ab_type, ab_rec.tm_queried);
                return ab_rec.ab_type;
            }
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: abonent %s: %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    return btUnknown;
}


} //cache
} //inman
} //smsc
