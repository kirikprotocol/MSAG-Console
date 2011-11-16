#ifndef __EYELINE_SS7NA_COMMON_LINKID_HPP__
# define __EYELINE_SS7NA_COMMON_LINKID_HPP__

# include <string>
# include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class LinkId {
public:
  LinkId() {}

  explicit LinkId(const std::string& linkIdValue)
    : _linkId(linkIdValue.c_str()) {}

  LinkId& operator=(const std::string& linkIdValue) {
    _linkId = linkIdValue.c_str();
    return *this;
  }

  LinkId& operator=(const LinkId& rhs) {
    if ( this != &rhs )
      _linkId = rhs._linkId;
    return *this;
  }

  bool operator==(const std::string& linkIdValue) const {
    if ( _linkId == linkIdValue.c_str() ) return true;
    else return false;
  }

  bool operator==(const LinkId& linkId) const {
    if ( _linkId == linkId._linkId ) return true;
    else return false;
  }

  bool operator!=(const std::string& linkIdValue) const {
    if ( _linkId != linkIdValue.c_str() ) return true;
    else return false;
  }

  bool operator!=(const LinkId& linkId) const {
    if ( _linkId != linkId._linkId ) return true;
    else return false;
  }

  bool operator<(const std::string& linkIdValue) const {
    if ( _linkId < linkIdValue.c_str() ) return true;
    else return false;
  }

  bool operator<(const LinkId& rhs) const {
    if ( _linkId < rhs._linkId ) return true;
    else return false;
  }

  const smsc::core::buffers::FixedLengthString<64>& getValue() const { return _linkId; }

  bool isEmpty() const { return _linkId.empty(); }

private:
  smsc::core::buffers::FixedLengthString<64> _linkId;
};

}}}

#endif
