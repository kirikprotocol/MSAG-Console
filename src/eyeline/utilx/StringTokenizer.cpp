#include "toLowerCaseString.hpp"
#include "StringTokenizer.hpp"

namespace eyeline {
namespace utilx {

StringTokenizer::StringTokenizer(const std::string& inputString)
  : _inputString(inputString), _token_beginning(0), _toLowerCaseConversion(false)
{
  if ( inputString.empty() )
    _token_beginning = std::string::npos;
}

bool
StringTokenizer::hasNextToken() const
{
  return _token_beginning != std::string::npos;
}

void
StringTokenizer::setDelimeterSymbols(const std::string& delimeterSymbols)
{
  _delimiters = delimeterSymbols;
}

std::string
StringTokenizer::nextToken()
{
  std::string::size_type forward;

  _token_beginning = _inputString.find_first_not_of(_delimiters, _token_beginning);
  forward = _inputString.find_first_of(_delimiters, _token_beginning);
  std::string foundLexem;
  if ( forward != std::string::npos )
    foundLexem = _inputString.substr(_token_beginning, forward - _token_beginning);
  else
    foundLexem = _inputString.substr(_token_beginning);

  _token_beginning = forward;
  _token_beginning = _inputString.find_first_not_of(_delimiters, _token_beginning);

  if ( _toLowerCaseConversion )
    return toLowerCaseString(foundLexem);
  else
    return foundLexem;
}

}}
