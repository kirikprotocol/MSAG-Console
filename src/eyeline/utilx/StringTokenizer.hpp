#ifndef __EYELINE_UTILX_STREAMTOKENIZER_HPP__
# define __EYELINE_UTILX_STREAMTOKENIZER_HPP__

# include <string>

namespace eyeline {
namespace utilx {

class StringTokenizer {
public:
  explicit StringTokenizer(const std::string& inputString);

  void setDelimeterSymbols(const std::string& delimeterSymbols);
  bool hasNextToken() const;
  std::string nextToken();

  void lowerCaseMode(bool on_off) { _toLowerCaseConversion = on_off; }
private:
  const std::string& _inputString;
  std::string::size_type _token_beginning;
  std::string _delimiters;
  bool _toLowerCaseConversion;
};

}}

#endif
