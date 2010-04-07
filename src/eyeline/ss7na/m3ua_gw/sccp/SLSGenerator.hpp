#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SLSGENERATOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SLSGENERATOR_HPP__

# include <sys/types.h>
# include "eyeline/utilx/Singleton.hpp"
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class SLSGenerator : public utilx::Singleton<SLSGenerator> {
public:
  uint8_t getNextSsl() {
    smsc::core::synchronization::MutexGuard synchronize(_lockForSlsGen);
    return ++_generatedSls % 15;
  }

private:
  SLSGenerator()
  : _generatedSls(0)
  {}
  friend class utilx::Singleton<SLSGenerator>;

  smsc::core::synchronization::Mutex _lockForSlsGen;
  uint8_t _generatedSls;
};

}}}}

#endif
