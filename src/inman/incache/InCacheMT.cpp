#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <assert.h>
#include "inman/incache/InCacheMT.hpp"

#define DIRECTORY_SEPARATOR '/'

namespace smsc {
namespace inman {
namespace cache {
/* ************************************************************************** *
 * class AbonentCacheMTR implementation:
 * ************************************************************************** */
// RAM usage per abonent:
//     XHash<>:Node * 2 +  AccessList:Node =
//     sizeof(void*) + ((sizeof(AbonentHashKey) + sizeof(XHashListLink<AbonentRecordRAM>))*2
//      + (sizeof(AbonentHashKey) + sizeof(void*)*2)

static const unsigned int DFLT_RAM = 40; //Mb
static const char _nmCch[] = "icache7.dat";

AbonentCacheMTR::AbonentCacheMTR(AbonentCacheCFG * cfg, Logger * uselog/* = NULL*/)
    : _cfg(*cfg)
{
    static const unsigned int DFLT_FACTOR =
        (sizeof(AbonentHashKey) + sizeof(AbonentRecordRAM) + 4 + sizeof(void*))*2
         + (sizeof(AbonentHashKey) + sizeof(void*)*3);

    assert(_cfg.nmDir.length() && _cfg.fileRcrd);
    logger = uselog ? uselog : Logger::getInstance("smsc.inman.InCache");
    uint32_t maxRamIt = ((((_cfg.RAM ? _cfg.RAM : DFLT_RAM)<<10)-1)/DFLT_FACTOR)<<10;
    smsc_log_info(logger, "InCache: RAM cache: %u abonents (factor: %u).",
                  maxRamIt, DFLT_FACTOR);
    ramCache.Init(_cfg.interval, maxRamIt);

    std::string nmFile(_cfg.nmDir);
    if (nmFile[nmFile.length()] != DIRECTORY_SEPARATOR)
        nmFile += DIRECTORY_SEPARATOR;
    nmFile += _nmCch;

    std::auto_ptr<FileCache> flCache(new FileCache());

    short   attNum = 2;
    bool    present = false;
    do {
        try {
            present = flCache->Open(nmFile.c_str(), _cfg.fileRcrd);
            attNum = 0;
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: %s", exc.what());
            //try to create new cache file
            if (--attNum && File::Exists(nmFile.c_str())) {
                std::string nmBad(nmFile);
                nmBad += ".broken";
                if (!rename(nmFile.c_str(), nmBad.c_str())) {
                    smsc_log_error(logger, "InCache: cache file renamed to %s", nmBad.c_str());
                } else {
                    flCache.reset();
                    attNum = 0;
                }
            }
        }
    } while (attNum);

    if (flCache.get()) {
        smsc_log_info(logger, "InCache: %s cache file %s(%u records of %u)",
                      present ? "red" : "created", nmFile.c_str(),
                      flCache->Used(), flCache->Size());
        fscMgr.reset(new FSCacheMonitor(flCache.release(), &ramCache, logger));
        fscMgr->Start();
    } else {
        smsc_log_info(logger, "InCache: proceeding in RAM only mode.");
    }
    return;
}

AbonentCacheMTR::~AbonentCacheMTR()
{
    if (fscMgr.get())
        fscMgr->Stop();
    smsc_log_info(logger, "InCache: closed.");
}
// ----------------------------------------
// AbonentCacheITF interface methods:
// ----------------------------------------
void AbonentCacheMTR::setAbonentInfo(const AbonentId & ab_number, const AbonentRecord & ab_rec)
{
    int  status = 0;
    bool toUpdate = true;//(bool)(ab_rec.ab_type != AbonentRecord::abtUnknown);
    {
        MutexGuard grd(ramCache.Sync());
        status = ramCache.Update(ab_number, ab_rec, toUpdate); //sets queried time
    }
    if (toUpdate && fscMgr.get())
        fscMgr->Update(ab_number);
//    smsc_log_debug(logger, "InCache: %s abonent(%s) %s, SCF %s",
//                    status ? "added" : "updated", ab_number.getSignals(), 
//                    ab_rec.type2Str(), ab_rec.gsmSCF.toString().c_str());
}

AbonentRecord::ContractType 
    AbonentCacheMTR::getAbonentInfo(const AbonentId & ab_number,
                                             AbonentRecord * p_ab_rec/* = NULL*/)
{
    {
        MutexGuard  guard(ramCache.Sync());
        AbonentRecordRAM *  rabRec = ramCache.LookUp(ab_number);
        if (rabRec && !rabRec->isExpired(_cfg.interval)) {
            if (p_ab_rec)
                *p_ab_rec = *(rabRec->getAbonentRecord());
            return rabRec->ab_type;
        }
    }
    AbonentHashData     ab_rec;
    if (fscMgr.get()) { //look in file cache
        try {
            if (fscMgr->LookUp(AbonentHashKey(ab_number), ab_rec)) {
                if (ab_rec.isExpired(_cfg.interval)) {
                    ab_rec.reset();
                } else if (ab_rec.ab_type != AbonentHashData::abtUnknown) {
                    MutexGuard  guard(ramCache.Sync());
                    ramCache.Update(ab_number, *ab_rec.getAbonentRecord());
                }
            }
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: getAbonent(%s): %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    if (p_ab_rec)
        *p_ab_rec = ab_rec;
    return ab_rec.ab_type;
}

/* ************************************************************************** *
 * class AbonentCacheMTR::RAMCache implementation:
 * ************************************************************************** */
AbonentCacheMTR::AbonentRecordRAM * 
    AbonentCacheMTR::RAMCache::LookUp(const AbonentId & ab_number)
{
    AbonentRecordRAM * pabRec = GetPtr(AbonentHashKey(ab_number));
    if (pabRec) {
//        accList.splice(accList.end(), accList, pabRec->accIt);
        accList.move_back(pabRec->accIt);
        pabRec->accIt = --accList.end();
        if (pabRec->isExpired(lfInterval))
            pabRec->ab_type = AbonentRecord::abtUnknown;
    }
    return pabRec;
}

//Returns: 0 - update, 1 - addition
int AbonentCacheMTR::RAMCache::Update(const AbonentId & ab_number,
                            const AbonentRecord & ab_rec, bool to_update/* = false*/)
{
    int status = 0; 
    AbonentHashKey      hkey(ab_number);
    AbonentRecordRAM *  pabRec = GetPtr(hkey);
    AbonentRecordRAM    ramRec(ab_rec, to_update); //accIt not inited yet!!!

    if (!pabRec) {  //insert
        makeSpace();
        status = XHash::Insert(hkey, ramRec);
        pabRec = GetPtr(hkey);
        accList.push_back(ab_number);
    } else {        //update
//        accList.splice(accList.end(), accList, pabRec->accIt);
        accList.move_back(pabRec->accIt);
        *pabRec = ramRec;
    }
    pabRec->accIt = --accList.end();
    return status;
}

void AbonentCacheMTR::RAMCache::makeSpace(void)
{
    if ((accList.Size() >= maxRamIt)) {
        AbonentsList::iterator it = accList.begin();
        while (it != accList.end()) {
            AbonentsList::iterator  cit = it++;
            AbonentHashKey          hkey(*cit);
            AbonentRecordRAM *      pabRec = GetPtr(hkey);
            if (!pabRec) //"RAMCache inconsistent! Fixing .."
                accList.erase(cit);
            else if ((pabRec->ab_type == AbonentRecordRAM::abtUnknown)
                     || !pabRec->toUpdate) {
                Delete(hkey);
                accList.erase(cit);
                return;
            }
        }
        //cache is fullfilled with records are to update;
        //delete the last accessed one
        Delete(AbonentHashKey(accList.front()));
        accList.pop_front();
    }
    return;
}

/* ************************************************************************** *
 * class AbonentCacheMTR::FSCacheMonitor implementation:
 * ************************************************************************** */
#define TIMEOUT_STEP 50 //millisecs
int AbonentCacheMTR::FSCacheMonitor::Execute(void)
{
    smsc_log_info(logger, "FSCache: started %s (%u records of %u)",
                  fsCache->FileName().c_str(), fsCache->Used(), fsCache->Size());
    _sync.Lock();
    _running = true;
    while (_running || !updList.empty()) {
        if (!updList.empty() && !_rehashOn) {
            do {
                AbonentId ab_number = *(updList.begin());
                _sync.Unlock();
                try {
                    AbonentRecordRAM *  prRec = NULL;
                    {
                        MutexGuard  grd(ramCache->Sync());
                        prRec = ramCache->LookUp(ab_number);
                    }
                    if (prRec) { //Update hashFile
                        MutexGuard grd(_sync);
                        AbonentHashData  abRec(*(prRec->getAbonentRecord()));
                        uint32_t r_num = fsCache->Insert(AbonentHashKey(ab_number), &abRec, true);
                        if (r_num) {
                            updList.pop_front();
                            smsc_log_debug(logger, "FSCache: rcd[%u.%u]: %s, %s, %s",
                                           fsCache->Size(), r_num, ab_number.getSignals(),
                                           abRec.type2Str(), abRec.tdpSCF.toString().c_str());
                            MutexGuard  grd(ramCache->Sync());
                            if ((prRec = ramCache->LookUp(ab_number)))
                                prRec->toUpdate = false;
                        } else {
                            _rehashOn = true;
                            fsCache->Flush();
                            smsc_log_info(logger, "FSCache: rehashing %s (%u records of %u) ..",
                                          fsCache->FileName().c_str(), fsCache->Used(), fsCache->Size());
                            rehasher.reset(new FileCache::HFRehasher(this, fsCache.get()));
                            rehasher->Start();
                            break;
                        }
                    }
                } catch (const std::exception& exc) {
                    smsc_log_error(logger, "FSCache: error on abonent(%s): %s",
                                   ab_number.getSignals(), exc.what());
                }
                _sync.Lock();
            } while (!updList.empty() && !_rehashOn);
        }
        _sync.wait(TIMEOUT_STEP); //unlocks and waits
    }
    _sync.Unlock();
    smsc_log_info(logger, "FSCache: stopped %s (%u records of %u)",
                  fsCache->FileName().c_str(), fsCache->Used(), fsCache->Size());
    return 0;
}

// -- HFRehashAcquirerITF implementation
void AbonentCacheMTR::FSCacheMonitor::onRehashDone(FileCache * new_hf, 
                                                   const char * error/* = NULL*/) _THROWS_NONE
{
    MutexGuard grd(_sync);
    _rehashOn = false;
    if (!new_hf) {
        smsc_log_error(logger, "FSCache: rehash failed: %s", error ? error : "unknown reason");
        smsc_log_error(logger, "FSCache: aborting, %u records lost ..", updList.Size());
        _running = false;
        updList.clear();
    } else {
        try {
            std::string nfName(fsCache->FileName());
            fsCache->Destroy();
            fsCache.reset(new_hf);
            fsCache->Rename(nfName.c_str());
            smsc_log_info(logger, "FSCache: rehashed %s (%u records of %u)",
                          fsCache->FileName().c_str(), fsCache->Used(), fsCache->Size());
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "FSCache: onRehash: %s", exc.what());
        }
    }
    _sync.notify();
    return;
}

} //cache
} //inman
} //smsc

