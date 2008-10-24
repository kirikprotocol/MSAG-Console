#include "Managers.h"
#include <scag/util/singleton/Singleton.h>

namespace scag { namespace transport { namespace mms {

using scag::util::singleton::SingletonHolder;

bool MmsManager::inited = false;
Mutex MmsManager::init_lock;

inline unsigned GetLongevity(MmsManager *) { return 251; };
typedef SingletonHolder<MmsManagerImpl> SingleMM;

MmsManager& MmsManager::Instance() {
  if (inited) {
    return SingleMM::Instance();
  }
  MutexGuard guard(init_lock);
  if(!inited) {
    throw std::runtime_error("MmsManager Not Inited!");
  }
}

void MmsManager::Init(MmsProcessor &p, const MmsManagerConfig &cfg, const char* mmscfgFile) {
  if (inited) {
    return;
  }
  MutexGuard guard(init_lock);
  if(!inited) {
    MmsManagerImpl& m = SingleMM::Instance();
    m.init(p, cfg, mmscfgFile);
    inited = true;
  }
}

MmsManagerImpl::MmsManagerImpl() : scags(*this), readers(*this), writers(*this),
                           rs_acceptor(*this, false), vasp_acceptor(*this, true), ConfigListener(MMSMAN_CFG) {

  logger = Logger::getInstance("mms.manager");
}

void MmsManagerImpl::configChanged() {
  //shutdown();
  //init(MmsProcessor::Instance(), ConfigManager::Instance().getMmsManConfig());
}

void MmsManagerImpl::init(MmsProcessor& p, const MmsManagerConfig& conf, const char* mmscfgFile) {
  cfg = conf;
  readers.init(cfg.reader_pool_size, cfg.reader_sockets, "mms.reader");
  writers.init(cfg.writer_pool_size, cfg.writer_sockets, "mms.writer");
  scags.init(cfg.scag_pool_size, cfg.scag_queue_limit, p);
  rs_acceptor.init(cfg.host.c_str(), cfg.rs_port);
  vasp_acceptor.init(cfg.host.c_str(), cfg.vasp_port);
  smsc_log_debug(logger, "connection_timeout=%d", cfg.connection_timeout);
}

void MmsManagerImpl::shutdown() {
  rs_acceptor.shutdown();
  vasp_acceptor.shutdown();

  while (!(readers.canStop() && writers.canStop() && scags.canStop()))
      sleep(1);

  scags.shutdown();
  readers.shutdown();
  writers.shutdown();
  smsc_log_info(logger, "MmsManager shutdown");
}

}//mms
}//transport
}//scag


