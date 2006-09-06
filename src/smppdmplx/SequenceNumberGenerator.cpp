#include "SequenceNumberGenerator.hpp"

smpp_dmplx::SequenceNumberGenerator::SeqGeneratorsMap_t
smpp_dmplx::SequenceNumberGenerator::_seqGeneratorsMap;

smpp_dmplx::SequenceNumberGenerator::SequenceNumberGenerator() : _nextSeq(0) {}

smpp_dmplx::SequenceNumberGenerator*
smpp_dmplx::SequenceNumberGenerator::getInstance(const std::string& systemId)
{
  SequenceNumberGenerator* instance;

  SeqGeneratorsMap_t::iterator iter = _seqGeneratorsMap.find(systemId);
  if ( iter != _seqGeneratorsMap.end() )
    instance = iter->second;
  else
    instance = _seqGeneratorsMap[systemId] = new SequenceNumberGenerator();

  return instance;
}

uint32_t smpp_dmplx::SequenceNumberGenerator::getNextValue()
{
  return ++_nextSeq;
}
