#ifndef __EYELINE_ASN1_BER_TESTS_TESTPATTERNSREGISTRY_HPP__
# define __EYELINE_ASN1_BER_TESTS_TESTPATTERNSREGISTRY_HPP__

# include <map>
# include <string>
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

class TestPatternsRegistry : public utilx::Singleton<TestPatternsRegistry> {
public:
  void insertResultPattern(const std::string& test_name, const std::string& tested_value,
                           const std::string& pattern_value);
  const std::string& getResultPattern(const std::string& test_name, const std::string& tested_value);

private:
  typedef std::map<std::string, std::string> patterns_reg_t;
  patterns_reg_t _patternsReg;
};

}}}}

#endif
