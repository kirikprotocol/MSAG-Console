#include "SequenceNumberGenerator.hpp"

namespace smpp_dmplx {

SequenceNumberGenerator::SeqGeneratorsMap_t
SequenceNumberGenerator::_seqGeneratorsMap;

SequenceNumberGenerator::SequenceNumberGenerator() : _nextSeq(0) {}

SequenceNumberGenerator*
SequenceNumberGenerator::getInstance(const std::string& systemId)
{
  SequenceNumberGenerator* instance;

  SeqGeneratorsMap_t::iterator iter = _seqGeneratorsMap.find(systemId);
  if ( iter != _seqGeneratorsMap.end() )
    instance = iter->second;
  else
    instance = _seqGeneratorsMap[systemId] = new SequenceNumberGenerator();

  return instance;
}

uint32_t
SequenceNumberGenerator::getNextValue()
{
  return ++_nextSeq;
}

}
