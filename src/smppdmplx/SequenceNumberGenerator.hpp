#ifndef __SMPPDMPLX_SEQUENCENUMBERGENERATOR_HPP__
# define __SMPPDMPLX_SEQUENCENUMBERGENERATOR_HPP__

# include <sys/types.h>
# include <string>
# include <map>

namespace smpp_dmplx {

class SequenceNumberGenerator {
public:
  static SequenceNumberGenerator* getInstance(const std::string& systemId);
  virtual uint32_t getNextValue();
protected:
  SequenceNumberGenerator();
private:
  typedef std::map<const std::string, SequenceNumberGenerator*> SeqGeneratorsMap_t;

  static SeqGeneratorsMap_t _seqGeneratorsMap;

  uint32_t _nextSeq;
};

}

#endif
