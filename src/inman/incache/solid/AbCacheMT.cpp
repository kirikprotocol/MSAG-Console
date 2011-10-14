#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#define DIRECTORY_SEPARATOR '/'

#include "inman/incache/solid/AbCacheMT.hpp"

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::inman::iaprvd::IAPProperty;
using smsc::inman::iaprvd::CSIRecord;

using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::packMAPAddress2OCTS;
using smsc::cvtutil::unpackOCTS2MAPAddress;
using smsc::cvtutil::packNumString2BCD;
using smsc::cvtutil::unpackBCD2NumString;


/* ************************************************************************** *
 * class AbonentCacheMTR::AbonentHashKey implementation:
 * ************************************************************************** */
AbonentCacheMTR::AbonentHashKey::AbonentHashKey(const AbonentId & ab_num) _THROWS_HFE
  : AbonentId(ab_num)
{
  bcdSz = packNumString2BCD(bcd, signals, length);
  if (bcdSz > _maxSize)
    throw smsc::util::Exception("AbonentHashKey(): invalid length %u", length);
}

uint32_t AbonentCacheMTR::AbonentHashKey::Read(File & fh, uint32_t max_octs/* = 0*/)
  _THROWS_HFE
{
  if (!max_octs || (max_octs > _maxSize))
    max_octs = _maxSize;
  //File::Read() throws if reads less bytes than requested
  bcdSz = (uint32_t)fh.Read(bcd, max_octs);
  int res = unpackBCD2NumString(bcd, signals, bcdSz);
  length = (res > 0) ? (uint8_t)res: 0;
  signals[length] = 0;
  if (!length)
    throw smsc::util::Exception("AbonentHashKey::Read(): bad char or zero length");

  //isdn international address only
  numPlanInd = typeOfNumber = 1;
  return bcdSz;
}

uint32_t AbonentCacheMTR::AbonentHashKey::HashCode(uint32_t attempt/* = 0*/) const
{
  uint32_t hcode = 0;
  for (uint32_t i = 0; i<= attempt; ++i)
    hcode = crc32(hcode, bcd, bcdSz);
  return hcode;
}

/* ************************************************************************** *
 * class AbonentCacheMTR::AbonentHashData implementation:
 * ************************************************************************** */
const CSIUid_e AbonentCacheMTR::AbonentHashData::_knownCSI[2] = {
  UnifiedCSI::csi_O_BC, UnifiedCSI::csi_MO_SM
};

uint8_t AbonentCacheMTR::AbonentHashData::getKnownCSIsNum(void) const
{
  uint8_t rval = 0;
  for (uint8_t i = 0; i < _maxCSIsNum; ++i) {
    const GsmSCFinfo * pScf = csiSCF.getSCFinfo(_knownCSI[i]);
    if (pScf && !pScf->empty())
      ++rval;
  }
  return rval;
}

uint32_t AbonentCacheMTR::AbonentHashData::WriteTimeT(File& fh, time_t val)
  _THROWS_HFE
{
  if (sizeof(time_t) > sizeof(uint32_t)) {
    fh.WriteNetInt64((uint64_t)val);
    return 8;
  }
  fh.WriteNetInt32((uint32_t)val);
  return 4;
}
time_t AbonentCacheMTR::AbonentHashData::ReadTimeT(File& fh)
  _THROWS_HFE
{
  time_t val;
  if (sizeof(time_t) > sizeof(uint32_t))
    val = (time_t)fh.ReadNetInt64();
  else
    val = (time_t)fh.ReadNetInt32();
  return val;
}

// -- HashFileEntityITF methods implementation
//Reads all CSIs data
uint32_t AbonentCacheMTR::AbonentHashData::Read(File & fh, uint32_t max_octs/* = 0*/)
  _THROWS_HFE
{
  uint32_t rv = sizeof(time_t) + 1;
  uint8_t fb = fh.ReadByte(); //[numSCF:6b | contract:2b]
  abType = static_cast<AbonentContract_e>(fb & 0x03);
  tmQueried = ReadTimeT(fh);

  csiSCF.clear();
  if (fb & 0xFC) {
    uint8_t cnt = (fb & 0xFC) >> 2;   //number of CSIs records

    for (; cnt; --cnt) {
      fb = fh.ReadByte(); ++rv; //[iapType:3b | CSIUid_e:5b]
      CSIUid_e csiId = UnifiedCSI::get(fb & 0x1F);
      
      if (csiId == UnifiedCSI::csi_UNDEFINED)
        break; //corrupted/unknown record

      //read SCF parms
      CSIRecord & csiRec = csiSCF[csiId];
      csiRec.csiId = csiId;
      csiRec.iapId = IAPProperty::val2Kind(fb >> 5);
      csiRec.scfInfo.serviceKey = (uint32_t)fh.ReadNetInt32();

      uint8_t len = fh.ReadByte();
      rv += 5;
      if (len && (len <= MAPConst::MAX_ISDN_AddressLength)) {
        uint8_t oct_buf[sizeof(TONNPI_ADDRESS_OCTS)];
        fh.Read(oct_buf, len);
        rv += len;
        unpackOCTS2MAPAddress(csiRec.scfInfo.scfAddress, oct_buf, len);
      }
    }
  }
  return rv;
}

//Writes only known CSIs data
uint32_t AbonentCacheMTR::AbonentHashData::Write(File & fh) const _THROWS_HFE
{
  uint32_t  sz = 1;
  uint8_t   numSCFs = getKnownCSIsNum();
  uint8_t   fb = (uint8_t)abType | (numSCFs << 2); //[numSCF:6b | contract:2b]

  fh.WriteByte(fb);
  sz += WriteTimeT(fh, tmQueried);
  if (!numSCFs)
    return sz;

  //Write known CSI records
  for (uint8_t i = 0; i < _maxCSIsNum; ++i) {
    const CSIRecord * csiRec = csiSCF.find(_knownCSI[i]);
    if (csiRec && !csiRec->scfInfo.empty()) {
      //[iapType:3b | CSIUid_e:5b]
      fb = (uint8_t)csiRec->csiId | ((uint8_t)csiRec->iapId << 5);
      fh.WriteByte(fb); ++sz;

      fh.WriteNetInt32(csiRec->scfInfo.serviceKey);
      sz += 4 + 1;    //serviceKey + address length

      uint8_t oct_buf[sizeof(TONNPI_ADDRESS_OCTS)];
      unsigned len = packMAPAddress2OCTS(csiRec->scfInfo.scfAddress, oct_buf);
      if ((len > 1) && (len <= MAPConst::MAX_ISDN_AddressLength)) {
        fh.WriteByte((uint8_t)len);
        fh.Write(oct_buf, len);
        sz += len;
      } else
        fh.WriteByte(0);
    }
  }
  return sz;
}


/* ************************************************************************** *
 * class AbonentCacheMTR implementation:
 * ************************************************************************** */
// RAM usage per abonent:
//     XHash<>:Node * 2 +  AccessList:Node =
//     sizeof(void*) + ((sizeof(AbonentHashKey) + sizeof(XHashListLink<AbonentRecordRAM>))*2
//      + (sizeof(AbonentHashKey) + sizeof(void*)*2)

static const unsigned int DFLT_RAM = 40; //Mb
static const char _nmCch[] = "icache7.dat";

AbonentCacheMTR::AbonentCacheMTR(const AbonentCacheCFG & use_cfg, Logger * uselog/* = NULL*/)
    : _cfg(use_cfg)
{
  static const unsigned int DFLT_FACTOR =
      (sizeof(AbonentHashKey) + sizeof(AbonentRecordRAM) + 4 + sizeof(void*))*2
       + (sizeof(AbonentHashKey) + sizeof(void*)*3);

  logger = uselog ? uselog : Logger::getInstance("smsc.inman.InCache");
  uint32_t maxRamIt = ((((_cfg.RAM ? _cfg.RAM : DFLT_RAM)<<10)-1)/DFLT_FACTOR)<<10;
  smsc_log_info(logger, "InCache: RAM cache: %u abonents (factor: %u).",
                maxRamIt, DFLT_FACTOR);
  ramCache.Init(maxRamIt);

  std::auto_ptr<FileCache> flCache;
  bool    present = false;    //cache file exists
  if (_cfg.nmDir.length() && _cfg.fileRcrd) {
    std::string nmFile(_cfg.nmDir);
    if (nmFile[nmFile.length()-1] != DIRECTORY_SEPARATOR)
      nmFile += DIRECTORY_SEPARATOR;
    nmFile += _nmCch;

    flCache.reset(new FileCache());
    short   attNum = 2;
    do {
      try {
        present = flCache->Open(nmFile.c_str(), _cfg.fileRcrd);
        attNum = 0;
      } catch (const std::exception & exc) {
        smsc_log_error(logger, "InCache: %s", exc.what());
        //try to create new cache file
        if (--attNum) {
          if (File::Exists(nmFile.c_str())) {
            std::string nmBad(nmFile);
            nmBad += ".broken";
            if (!rename(nmFile.c_str(), nmBad.c_str())) {
              smsc_log_error(logger, "InCache: cache file renamed to %s", nmBad.c_str());
            } else {
              flCache.reset();
              attNum = 0;
            }
          }
        } else
          flCache.reset();
      }
    } while (attNum);
  }

  if (flCache.get()) {
    smsc_log_info(logger, "InCache: %s cache file %s",
                  present ? "red" : "created", flCache->Details().c_str());
    fscMgr.reset(new FSCacheMonitor(flCache.release(), &ramCache, logger));
  } else {
    smsc_log_info(logger, "InCache: proceeding in RAM only mode.");
  }
  return;
}

AbonentCacheMTR::~AbonentCacheMTR()
{
  Stop();
  smsc_log_info(logger, "InCache: closed.");
}
//Starts file cache activity
bool AbonentCacheMTR::Start(void)
{
  return fscMgr.get() ? fscMgr->Start() : true;
}
//Stops file cache activity
void AbonentCacheMTR::Stop(bool do_wait/* = false*/)
{
  if (fscMgr.get())
    fscMgr->Stop(do_wait);
}

// ----------------------------------------
// AbonentCacheITF interface methods:
// ----------------------------------------
void AbonentCacheMTR::setAbonentInfo(const AbonentId & ab_number,
                                        const AbonentSubscription & ab_rec)
{
//    int  status = 
    ramCache.Update(ab_number, ab_rec); //sets queried time
//    smsc_log_debug(logger, "InCache: %s abonent(%s) %s, SCF %s",
//                    status ? "added" : "updated", ab_number.getSignals(), 
//                    ab_rec.type2Str(), ab_rec.gsmSCF.toString().c_str());
    if (fscMgr.get())
        fscMgr->Awake();
}

AbonentContract_e
    AbonentCacheMTR::getAbonentInfo(const AbonentId & ab_number,
                                    AbonentSubscription * p_ab_rec/* = NULL*/,
                                    uint32_t exp_timeout/* = 0*/)
{
    AbonentHashData abData; //abtUnknown
    if (!p_ab_rec)
        p_ab_rec = &abData.abRecord();
    if (!exp_timeout)
        exp_timeout = _cfg.interval;
    if (ramCache.LookUp(ab_number, *p_ab_rec, exp_timeout))
        return p_ab_rec->abType;

    if (fscMgr.get()) { //look in file cache
        try {
            if (fscMgr->LookUp(AbonentHashKey(ab_number), abData)) {
                if (abData.abRecord().isExpired(exp_timeout)) {
                    abData.abRecord().clear();
                } else if (abData.abRecord().isUnknown()) {
                    ramCache.Update(ab_number, abData.abRecord());
                }
            }
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "InCache: getAbonent(%s): %s",
                            ab_number.getSignals(), exc.what());
        }
    }
    *p_ab_rec = abData.abRecord();
    return p_ab_rec->abType;
}

/* ************************************************************************** *
 * class AbonentCacheMTR::RAMCache implementation:
 * ************************************************************************** */
bool AbonentCacheMTR::RAMCache::LookUp(const AbonentId & ab_number,
                                    AbonentSubscription & ab_rec, uint32_t exp_timeout/* = 0*/)
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
                            const AbonentSubscription & ab_rec)
{
    MutexGuard grd(rcSync);
    int status = 0; 
    AbonentHashKey      hkey(ab_number);
    AbonentRecordRAM *  pabRec = GetPtr(hkey);
    AbonentRecordRAM    ramRec(ab_rec, true); //accIt not inited yet!!!

    if (!pabRec) {  //insert
        makeSpace();
        status = this->Insert(hkey, ramRec);
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
                                           AbonentSubscription & ab_rec)
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
            } else if (!pabRec->marked || pabRec->isUnknown()) {
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
//Takes ownership of fs_cache!!!
AbonentCacheMTR::FSCacheMonitor::FSCacheMonitor(
  FileCache * fs_cache, RAMCache *ram_cache, Logger * use_log/* = NULL*/)
  : _rehashOn(false), _running(false), _rehashMode(true), ramCache(ram_cache)
{ 
  logger = use_log ? use_log : Logger::getInstance("smsc.inman.InCache");
  fsCache.reset(fs_cache);
  _rehashMode = fsCache->rehashAllowed();
  if (!_rehashMode) {
    smsc_log_info(logger, "FSCache: rehash mode is OFF for %s",
                  fsCache->Details().c_str());
  }
}
AbonentCacheMTR::FSCacheMonitor::~FSCacheMonitor()
{
  Stop(true);
  Thread::WaitFor();
  MutexGuard  grd(_sync);
  try {
    fsCache->Close();
    smsc_log_info(logger, "FSCache: closed %s", fsCache->Details().c_str());
  } catch (const std::exception & exc) {
    smsc_log_error(logger, "FSCache: %s", exc.what());
  }
}

bool AbonentCacheMTR::FSCacheMonitor::Start(void)
{
  if (isRunning())
    return true;
  smsc_log_debug(logger, "FSCache: starting ..");
  Thread::Start();
  {
    MutexGuard grd(_sync);
    if (_running)
      return true;
    _sync.wait(FSC_TIMEOUT_STEP);
    if (!_running) {
      smsc_log_fatal(logger, "FSCache: unable to start");
      return false;
    }
  }
  return true;
}

void AbonentCacheMTR::FSCacheMonitor::Stop(bool do_wait/* = false*/)
{
  {
    MutexGuard  grd(_sync);
    if (_running) {
      _rehashMode = _running = false;
      smsc_log_debug(logger, "FSCache: stopping ..");
    }
    if (_rehashOn) {
      smsc_log_debug(logger, "FSCache: stopping rehasher ..");
      rehasher->Stop();
      //wait for rehasher report
      while (_rehashOn)
        _sync.wait(FSC_TIMEOUT_STEP);
    }
    _sync.notify();
  }
  if (do_wait)
    Thread::WaitFor();
}

#define TIMEOUT_STEP 50 //millisecs
int AbonentCacheMTR::FSCacheMonitor::Execute(void)
{
    smsc_log_info(logger, "FSCache: started %s", fsCache->Details().c_str());
    
    _sync.Lock();
    _running = true;
    while (_running) {
        AbonentId       abNum;
        AbonentHashData abRec;

        if (!_rehashOn) {
            _sync.Unlock();
            while (ramCache->NextMarked(abNum, abRec.abRecord())) {
                MutexGuard grd(_sync);
                try {
                    uint32_t r_num = fsCache->Insert(AbonentHashKey(abNum), &abRec, true);
                    if (r_num) {
                        smsc_log_debug(logger, "FSCache: rcd[%u.%u]: %s, %s",
                                           fsCache->Size(), r_num, abNum.getSignals(),
                                           abRec.toString().c_str());
                    } else if (!r_num && _rehashMode) {
                        _rehashMode = fsCache->rehashAllowed();
                        if (!_rehashMode) {
                            smsc_log_info(logger, "FSCache: rehash mode is OFF for %s",
                                            fsCache->Details().c_str());
                            smsc_log_debug(logger, "FSCache: ignored: %s, %s",
                                            abNum.getSignals(), abRec.toString().c_str());
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
                        smsc_log_debug(logger, "FSCache: ignored: %s, %s",
                                        abNum.getSignals(), abRec.toString().c_str());
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

