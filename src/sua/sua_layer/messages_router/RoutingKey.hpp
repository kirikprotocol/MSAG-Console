#ifndef __SUA_SUALAYER_MESSAGESROUTER_ROUTINGKEY_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_ROUTINGKEY_HPP__ 1

# include <string.h>
# include <string>
# include <sua/communication/LinkId.hpp>

namespace messages_router {

class RoutingKey {
public:
  RoutingKey() :
    _isSetDestinationGT(false), _isSetIncomingLinkId(false), _isSetDestinationSsn(false) {}

  void setDestinationGT(const std::string& dstGT) {
    _destinationGT = dstGT; _isSetDestinationGT = true;
  }
  bool isSetDestinationGT() const { return _isSetDestinationGT; }
  const std::string& getDestinationGT() const { return _destinationGT; }

  void setIncomingLinkId(const communication::LinkId& linkId) {
    _incomingLinkId = linkId; _isSetIncomingLinkId = true;
  }
  bool isSetIncomingLinkId() const { return _isSetIncomingLinkId; }
  communication::LinkId getIncomingLinkId() const { return _incomingLinkId; }

  void setDestinationSSN(uint8_t ssn) {
    _destinationSsn = ssn; _isSetDestinationSsn = true;
  }
  bool isSetDestinationSSN() const { return _isSetDestinationSsn; }
  uint8_t getDestinationSSN() const { return _destinationSsn; }

  std::string toString() const {
    char strBuf[128];
    int pos=0;
    if ( _isSetDestinationGT )
      pos += snprintf(strBuf, sizeof(strBuf), "destinationGT=%s,", _destinationGT.c_str());
    if ( _isSetDestinationSsn )
      pos += snprintf(strBuf + pos, sizeof(strBuf) - pos, "destinationSSN=%d,", _destinationSsn);
    if ( _isSetIncomingLinkId )
      pos += snprintf(strBuf + pos, sizeof(strBuf) - pos, "incomingLinkId=%s,", _incomingLinkId.getValue().c_str());
    strBuf[pos-1] = 0;
    return std::string(strBuf);
  }
private:
  std::string _destinationGT;
  communication::LinkId _incomingLinkId;
  uint8_t _destinationSsn;
  bool _isSetDestinationGT, _isSetIncomingLinkId, _isSetDestinationSsn;
};

}

#endif
