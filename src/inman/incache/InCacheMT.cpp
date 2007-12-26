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
    ramCache.Init(maxRamIt);

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
        smsc_log_info(logger, "InCache: %s cache file %s",
                      present ? "red" : "created", flCache->Details().c_str());
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
void AbonentCacheMTR::setAbonentInfo(const AbonentId & ab_number,
                                        const AbonentRecord & ab_rec)
{
    int  status = ramCache.Update(ab_number, ab_rec); //sets queried time
//    smsc_log_debug(logger, "InCache: %s abonent(%s) %s, SCF %s",
//                    status ? "added" : "updated", ab_number.getSignals(), 
//                    ab_rec.type2Str(), ab_rec.gsmSCF.toString().c_str());
    if (fscMgr.get())
        fscMgr->Awake();
}

AbonentRecord::ContractType 
    AbonentCacheMTR::getAbonentInfo(const AbonentId & ab_number,
                                    AbonentRecord * p_ab_rec/* = NULL*/,
                                    uint32_t exp_timeout/* = 0*/)
{
    AbonentHashData abData; //abtUnknown
    if (!p_ab_rec)
        p_ab_rec = &abData.abRecord();
    if (!exp_timeout)
        exp_timeout = _cfg.interval;
    if (ramCache.LookUp(ab_number, *p_ab_rec, exp_timeout))
        return p_ab_rec->ab_type;

    if (fscMgr.get()) { //look in file cache
        try {
            if (fscMgr->LookUp(AbonentHashKey(ab_number), abData)) {
                if (abData.abRecord().isExpired(exp_timeout)) {
                    abData.abRecord().reset();
                } else if (abData.abRecord().ab_type != AbonentRecord::abtUnknown) {
                    ramCache.Update(ab_number, abData.abRecord());
                }
            }
        } catch (std::exception & exc) {
            smsc_log_error(logger, "InCache: getAbonent(%s): %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    *p_ab_rec = abData.abRecord();
    return p_ab_rec->ab_type;
}

/* ************************************************************************** *
 * class AbonentCacheMTR::RAMCache implementation:
 * ************************************************************************** */
bool AbonentCacheMTR::RAMCache::LookUp(const AbonentId & ab_number,
                                    AbonentRecord & ab_rec, uint32_t exp_timeout/* = 0*/)
{
    MutexGuard grd(rcSync);
    AbonentHashKey      abNum(ab_number);
    AbonentRecordRAM *  pabRec = GetPtr(abNum);
    if (!pabRec)
        return false;

    if (exp_timeout && pabRec->isExpired(exp_timeout)) {
        accList.erase(pabRec->accIt);
        Delete(abNum);
        return false;
    }
    accList.move_back(pabRec->accIt);
    pabRec->accIt = --accList.end();
    ab_rec = pabRec->abRecord();
    return true;
}

bool AbonentCacheMTR::RAMCache::Mark(const AbonentId & ab_number)
{
    MutexGuard grd(rcSync);
    AbonentRecordRAM * pabRec = GetPtr(AbonentHashKey(ab_number));
    return pabRec ? (pabRec->marked = true) : false;
}

//Returns: 0 - update, 1 - addition
int AbonentCacheMTR::RAMCache::Update(const AbonentId & ab_number,
                            const AbonentRecord & ab_rec)
{
    MutexGuard grd(rcSync);
    int status = 0; 
    AbonentHashKey      hkey(ab_number);
    AbonentRecordRAM *  pabRec = GetPtr(hkey);
    AbonentRecordRAM    ramRec(ab_rec, true); //accIt not inited yet!!!

    if (!pabRec) {  //insert
        makeSpace();
        status = XHash::Insert(hkey, ramRec);
        pabRec = GetPtr(hkey);
        accList.push_back(ab_number);
    } else {        //update
        accList.move_back(pabRec->accIt);
        *pabRec = ramRec;
    }
    pabRec->accIt = --accList.end();
    updList.push_back(ab_number);
    return status;
}

bool AbonentCacheMTR::RAMCache::NextMarked(AbonentId & ab_num,
                                           AbonentRecord & ab_rec)
{
    MutexGuard grd(rcSync);
    if (!updList.empty()) {
        AbonentHashKey  abNum(updList.front());
        AbonentRecordRAM * pabRec = GetPtr(abNum);

        updList.pop_front();
        if (pabRec && pabRec->marked) {
            ab_num = (AbonentId)abNum;
            ab_rec = pabRec->abRecord();
            pabRec->marked = false;
            return true;
        }
    }
    return false;
}

void AbonentCacheMTR::RAMCache::makeSpace(void)
{
    if ((accList.Size() >= maxRamIt)) {
        uint32_t    attNum = maxSpaceAtt;
        AbonentsList::iterator it = accList.begin();

        while (it != accList.end()) {
            AbonentsList::iterator  cit = it++;
            AbonentHashKey          hkey(*cit);
            AbonentRecordRAM *      pabRec = GetPtr(hkey);
            if (!pabRec) {
            //record was deleted from hash: it was a previous 'victim'
                accList.erase(cit);
            } else if (!pabRec->marked ||
                       (pabRec->ab_type == AbonentRecordRAM::abtUnknown)) {
                Delete(hkey);
                accList.erase(cit);
                return;
            }
            if (!--attNum)
                break;
        }
        //cache is fullfilled with marked records;
        //just delete the last accessed one.
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
    smsc_log_info(logger, "FSCache: started %s", fsCache->Details().c_str());
    
    _sync.Lock();
    _running = true;
    while (_running) {
        bool            doUpd = false;
        AbonentId       abNum;
        AbonentHashData abRec;

        if (!_rehashOn) {
            _sync.Unlock();
            while (ramCache->NextMarked(abNum, abRec.abRecord())) {
                MutexGuard grd(_sync);
                try {
                    uint32_t r_num = fsCache->Insert(AbonentHashKey(abNum), &abRec, true);
                    if (r_num) {
                        smsc_log_debug(logger, "FSCache: rcd[%u.%u]: %s, %s, %s",
                                           fsCache->Size(), r_num, abNum.getSignals(),
                                           abRec.type2Str(), abRec.tdpSCF.toString().c_str());
                    } else if (!r_num && _rehashMode) {
                        _rehashMode = fsCache->rehashAllowed();
                        if (!_rehashMode) {
                            smsc_log_info(logger, "FSCache: rehash mode is OFF for %s",
                                            fsCache->Details().c_str());
                            smsc_log_debug(logger, "FSCache: ignored: %s, %s, %s",
                                            abNum.getSignals(), abRec.type2Str(),
                                            abRec.tdpSCF.toString().c_str());
                        } else {
                            ramCache->Mark(abNum); //mark back abonent
                            _rehashOn = true;
                            fsCache->Flush();
                            rehasher.reset(new FileCache::HFRehasher(this, fsCache.get()));
                            smsc_log_info(logger, "FSCache: rehashing %s ..",
                                           fsCache->Details().c_str());
                            rehasher->Start();
                            break; //sleep and wait for onRehashDone()
                        }
                    } else
                        smsc_log_debug(logger, "FSCache: ignored: %s, %s, %s",
                                        abNum.getSignals(), abRec.type2Str(),
                                        abRec.tdpSCF.toString().c_str());
                } catch (const std::exception& exc) {
                    smsc_log_error(logger, "FSCache: exception on abonent(%s): %s",
                                   abNum.getSignals(), exc.what());
                    _rehashOn = false;
                }
            }
            _sync.Lock();
        }
        _sync.wait(TIMEOUT_STEP); //unlocks and waits
    }
    _sync.Unlock();
    smsc_log_info(logger, "FSCache: stopped %s", fsCache->Details().c_str());
    return 0;
}

// -- HFRehashAcquirerITF implementation
void AbonentCacheMTR::FSCacheMonitor::onRehashDone(FileCache * new_hf, 
                                                   const char * error/* = NULL*/) _THROWS_NONE
{
    MutexGuard grd(_sync);
    _rehashOn = false;
    if (!new_hf) {
        smsc_log_error(logger, "FSCache: rehash failed: %s",
                       error ? error : "unknown reason");
        //no more new abonents, only looking for and update of existing ones
        _rehashMode = false;
        smsc_log_info(logger, "FSCache: rehash mode is OFF for %s",
                      fsCache->Details().c_str());
    } else {
        try {
            std::string nfName(fsCache->FileName());
            fsCache->Destroy();
            fsCache.reset(new_hf);
            fsCache->Rename(nfName.c_str());
            smsc_log_info(logger, "FSCache: rehashed %s", fsCache->Details().c_str());
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "FSCache: onRehashDone: %s", exc.what());
        }
    }
    _sync.notify();
    return;
}

} //cache
} //inman
} //smsc

