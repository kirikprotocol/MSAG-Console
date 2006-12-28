#ifndef __SMSC_INFO_SME_INFOSME_T_SEARCHCRITERION_HPP__
# define __SMSC_INFO_SME_INFOSME_T_SEARCHCRITERION_HPP__ 1

# include <time.h>
# include <string>

namespace smsc{ namespace infosme
{

class InfoSme_T_SearchCriterion
{
public:
  InfoSme_T_SearchCriterion() : _isSetAbonetAdress(false), _isSetState(false), _isSetFromDate(false), _isSetToDate(false), _isSetOrderByColumnName(false), _isSetMsgLimit(false) {}
  void setAbonentAddress(const std::string& address) {
    _isSetAbonetAdress=true;
    _abonentAddress = address;
  }
  bool isSetAbonentAddress() const { return _isSetAbonetAdress; }
  const std::string& getAbonentAddress() const { return _abonentAddress; }

  void setState(uint8_t messageState) {
    _isSetState = true;
    _state = messageState;
  }
  bool isSetState() const { return _isSetState; }
  uint8_t getState() const { return _state; }

  void setFromDate(time_t unixTime) {
    _isSetFromDate = true;
    _fromDate = unixTime;
  }
  bool isSetFromDate() const { return _isSetFromDate; }
  time_t getFromDate() const { return _fromDate; }

  void setToDate(time_t unixTime) {
    _isSetToDate = true;
    _toDate = unixTime;
  }
  bool isSetToDate() const { return _isSetToDate; }
  time_t getToDate() const { return _toDate; }

  void setOrderByCriterion(const std::string& orderByColumnName) {
    _isSetOrderByColumnName = true;
    _orderByColumnName = orderByColumnName;
  }
  bool isSetOrderByCriterion() const { return _isSetOrderByColumnName; }
  const std::string& getOrderByCriterion() const { return _orderByColumnName; }

  void setOrderDirection(const std::string& orderDirection) {
    _isSetOrderDirection = true;
    _orderDirection = orderDirection;
  }
  bool isSetOrderDirection() const { return _isSetOrderDirection; }
  const std::string& getOrderDirection() const { return _orderDirection; }

  void setMsgLimit(size_t msgLimit) {
    _isSetMsgLimit = true;
    _msgLimit = msgLimit;
  }
  bool isSetMsgLimit() const { return _isSetMsgLimit; }
  size_t getMsgLimit() const { return _msgLimit; }

private:
  bool _isSetAbonetAdress;
  std::string _abonentAddress;

  bool _isSetState;
  uint8_t _state;

  bool _isSetFromDate;
  time_t _fromDate;

  bool _isSetToDate;
  time_t _toDate;

  bool _isSetOrderByColumnName;
  std::string _orderByColumnName;

  bool _isSetOrderDirection;
  std::string _orderDirection;

  bool _isSetMsgLimit;
  size_t _msgLimit;
};

}
}
#endif
