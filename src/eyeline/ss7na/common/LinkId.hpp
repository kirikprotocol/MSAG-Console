#ifndef __EYELINE_SS7NA_COMMON_LINKID_HPP__
# define __EYELINE_SS7NA_COMMON_LINKID_HPP__

# include <string>

namespace eyeline {
namespace ss7na {
namespace common {

class LinkId {
public:
  LinkId() {}

  explicit LinkId(const std::string& linkIdValue)
    : _linkId(linkIdValue) {}

  LinkId& operator=(const std::string& linkIdValue) {
    _linkId = linkIdValue;
    return *this;
  }

  LinkId& operator=(const LinkId& rhs) {
    if ( this != &rhs )
      _linkId = rhs._linkId;
    return *this;
  }

  bool operator==(const std::string& linkIdValue) const {
    if ( _linkId == linkIdValue ) return true;
    else return false;
  }

  bool operator==(const LinkId& linkId) const {
    if ( _linkId == linkId._linkId ) return true;
    else return false;
  }

  bool operator!=(const std::string& linkIdValue) const {
    if ( _linkId != linkIdValue ) return true;
    else return false;
  }

  bool operator!=(const LinkId& linkId) const {
    if ( _linkId != linkId._linkId ) return true;
    else return false;
  }

  bool operator<(const std::string& linkIdValue) const {
    if ( _linkId < linkIdValue ) return true;
    else return false;
  }

  bool operator<(const LinkId& rhs) const {
    if ( _linkId < rhs._linkId ) return true;
    else return false;
  }

  bool operator>(const std::string& linkIdValue) const {
    if ( _linkId > linkIdValue ) return true;
    else return false;
  }

  bool operator>(const LinkId& rhs) const {
    if ( _linkId > rhs._linkId ) return true;
    else return false;
  }

  const std::string& getValue() const { return _linkId; }

  bool isEmpty() const { return _linkId.empty(); }

private:
  std::string _linkId;
};

}}}

#endif
