#include "TestPatternsRegistry.hpp"
#include "eyeline/utilx/Exception.hpp"
#include <utility>

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

void
TestPatternsRegistry::insertResultPattern(const std::string& test_name, const std::string& tested_value,
                                          const std::string& pattern_value)
{
  const std::string& keyValue= test_name + tested_value;
  std::pair<patterns_reg_t::iterator, bool> res= _patternsReg.insert(std::make_pair(keyValue, pattern_value));
  if (!res.second)
    throw utilx::DuplicatedRegistryKeyException("TestPatternsRegistry::insertResultPattern::: key='%s' already registered",
                                                keyValue.c_str());
}

const std::string&
TestPatternsRegistry::getResultPattern(const std::string& test_name, const std::string& tested_value)
{
  const std::string& keyValue= test_name + tested_value;
  patterns_reg_t::const_iterator iter= _patternsReg.find(keyValue);
  if (iter == _patternsReg.end())
    throw utilx::RegistryKeyNotFound("TestPatternsRegistry::getResultPattern::: key='%s' not found",
                                     keyValue.c_str());
  return iter->second;
}

}}}}
