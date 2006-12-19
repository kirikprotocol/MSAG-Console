#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <assert.h>
#include "inman/incache.hpp"

using smsc::core::buffers::HashListLink;

#define DIRECTORY_SEPARATOR '/'

namespace smsc {
namespace inman {
namespace cache {
/* ************************************************************************** *
 * class AbonentCache implementation:
 * ************************************************************************** */
// RAM usage per abonent:
//     Hash<>:Node * 2 +  AccessList:Node =
//     sizeof(void*) + ((sizeof(AbonentId) + sizeof(HashListLink<AbonentRecordRAM>))*2
//      + (sizeof(AbonentId) + sizeof(void*)*2)

static const unsigned int DFLT_RAM = 40; //Mb
static const char _nmCch[] = "icache7.dat";

AbonentCache::AbonentCache(AbonentCacheCFG * cfg, Logger * uselog/* = NULL*/)
    : _cfg(*cfg), useFile(false)
{
    static const unsigned int DFLT_FACTOR =
        (sizeof(AbonentId) + sizeof(AbonentRecordRAM) + 4 + sizeof(void*))*2
         + (sizeof(AbonentId) + sizeof(void*)*3);

    assert(_cfg.nmDir.length() && _cfg.fileRcrd);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.InCache");
    maxRamIt = ((((_cfg.RAM ? _cfg.RAM : DFLT_RAM)<<10)-1)/DFLT_FACTOR)<<10;
    smsc_log_info(logger, "InCache: RAM cache: %u abonents (factor: %u).",
                  maxRamIt, DFLT_FACTOR);

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

bool AbonentCache::load(const char * file_nm)
{
    try {
        bool present = File::Exists(file_nm);
        if (present)
            flCache.Open(file_nm, false, false);
        else
            flCache.Create(file_nm, _cfg.fileRcrd, false);
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
int AbonentCache::ramInsert(const AbonentId & ab_number,
                            const AbonentRecord & ab_rec)
{
    int status = 0; 
    AbonentRecordRAM    ramRec(ab_rec);
    AbonentRecordRAM *  pabRec = cache.GetPtr(ab_number.getSignals());

    if (!ramRec.tm_queried)
        ramRec.tm_queried = time(NULL);

    if (!pabRec) {
        if (accList.size() >= maxRamIt) {
            cache.Delete(accList.back().getSignals());
            accList.pop_back();
        }
        status = cache.Insert(ab_number.getSignals(), ramRec);
        pabRec = cache.GetPtr(ab_number.getSignals());
    } else { //update
        *pabRec = ramRec;
        accList.erase(pabRec->accIt);
    }
    accList.push_front(ab_number);
    pabRec->accIt = accList.begin();
    return status;
}

AbonentRecord * AbonentCache::ramLookUp(AbonentId & ab_number)
{
    AbonentRecordRAM * pabRec = cache.GetPtr(ab_number.getSignals());
    if (pabRec) {
        if (pabRec->accIt != accList.begin()) {
            accList.erase(pabRec->accIt);
            accList.push_front(ab_number);
            pabRec->accIt = accList.begin();
        }
        if (!pabRec->isUnknown()) {
            if (pabRec->isExpired(_cfg.interval)) {
                pabRec->ab_type = AbonentRecord::abtUnknown;
                smsc_log_debug(logger, "InCache: abonent %s info is expired",
                               ab_number.getSignals());
            }
        }
    }
    return (AbonentRecord *)pabRec;
}

// ----------------------------------------
// AbonentCacheITF interface methods:
// ----------------------------------------
void AbonentCache::setAbonentInfo(const AbonentId & ab_number,
                                  const AbonentRecord & ab_rec)
{
    MutexGuard grd(cacheGuard);
    int status = ramInsert(ab_number, ab_rec); //sets queried time

    if (ab_rec.ab_type != AbonentRecord::abtUnknown) {
        try {
            flCache.Insert(AbonentHashKey(ab_number), AbonentHashData(ab_rec), true);
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: abonent %s: %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    smsc_log_debug(logger, "InCache: abonent %s is %s: %s, SCF %s",
                    ab_number.getSignals(), status ? "added" : "updated",
                    ab_rec.type2Str(), ab_rec.gsmSCF.toString().c_str());
}

AbonentBillType AbonentCache::getAbonentInfo(AbonentId & ab_number,
                                             AbonentRecord * p_ab_rec/* = NULL*/)
{
    MutexGuard  guard(cacheGuard);
    AbonentHashData     ab_rec;
    AbonentRecord *     rabRec = ramLookUp(ab_number);

    if (!rabRec || rabRec->isUnknown()) { //look in file cache
        try {
            if (flCache.LookUp(AbonentHashKey(ab_number), ab_rec)) {
                if (!ab_rec.isExpired(_cfg.interval))
                    ramInsert(ab_number, *(rabRec = ab_rec.getAbonentRecord()));
            }
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: abonent %s: %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    if (rabRec && p_ab_rec)
        *p_ab_rec = *rabRec;
    return rabRec ? rabRec->ab_type : AbonentRecord::abtUnknown;
}

} //cache
} //inman
} //smsc
