#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_DELIMITER_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_DELIMITER_HPP__

# include <sys/types.h>

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

class Descriminator : public utilx::Singleton<Descriminator> {
public:
/*  Descriminator()
  : _isBufferOnHeap(false), _localPointCodes(_bufferOnStack),
    _totalNumOfLPCs(0), _maxNumOfLPCs(DEFAULT_MAX_NUM_OF_PC)
  {}

  ~Descriminator() {
    if (_isBufferOnHeap)
      delete [] _localPointCodes;
  }
*/
  // return true if dpc for me
  bool checkRouteTermination(uint32_t dpc) const;
private:
  Descriminator() {}
  friend class utilx::Singleton<Descriminator>;
/*  void addLocalPointCode(uint32_t pc);

private:
  enum {DEFAULT_MAX_NUM_OF_PC=32};
  uint32_t _bufferOnStack[DEFAULT_MAX_NUM_OF_PC];
  uint32_t* _localPointCodes;
  bool _isBufferOnHeap;
  unsigned _totalNumOfLPCs, _maxNumOfLPCs;
  */
};

}}}}}

#endif
