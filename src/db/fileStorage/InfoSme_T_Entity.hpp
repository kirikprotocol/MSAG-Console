#ifndef __DBENTITYSTORAGE_INFOSME_T_ENTITY_HPP__
# define __DBENTITYSTORAGE_INFOSME_T_ENTITY_HPP__ 1

# include <stdio.h>
# include <sys/types.h>
# include <time.h>
# include <string>
# include <sstream>
# include "EntityKey.hpp"

class InfoSme_T_Entity {
public:
  InfoSme_T_Entity() :
    _id(0), _state(0), _abonentAddress(""), _sendDate(0), _message(""), _regionId("") {}
  InfoSme_T_Entity(uint64_t id,
                   uint8_t state,
                   const std::string& abonentAddress,
                   time_t   sendDate,
                   const std::string& message="",
                   const std::string& regionId="") :
    _id(id), _state(state), _abonentAddress(abonentAddress),
    _sendDate(sendDate), _message(message), _regionId(regionId) {}

  uint64_t getId() const { return _id; }
  uint8_t getState() const { return _state; }
  const std::string& getAbonentAddress() const { return _abonentAddress; }
  time_t getSendDate() const { return _sendDate; }
  const std::string& getMessage() const { return _message; }
  const std::string& getRegionId() const { return _regionId; }

  std::string toString() const {
    std::ostringstream obuf;
    obuf << "id=[" << _id
         << "],state=[" << static_cast<unsigned int>(_state)
         << "],abonentAddress=[" << _abonentAddress
         << "],sendDate=[" << _sendDate
         << "],message=[" << _message
         << "],regionId=[" << _regionId
         << "]";
    return obuf.str();
  }

  // entity key definitions
  class Id_Key
    : public EntityKey<key_traits<Id_Key> > {
  public:
    Id_Key(uint64_t idValue) : _idValue(idValue) {}
    // It's constructor for making Id_Key object from data of
    // InfoSme_T object.
    Id_Key(const InfoSme_T_Entity& dataValue) : _idValue (dataValue.getId()) {}

    virtual bool operator== (const Id_Key& rhs) const {
      if ( _idValue == rhs._idValue ) return true;
      else return false;
    }

    virtual bool operator< (const Id_Key& rhs) const {
      if ( _idValue < rhs._idValue ) return true;
      else return false;
    }

    virtual std::string toString() const {
      char str[64];
      snprintf(str,sizeof(str),"idValue=[%lld]",_idValue);
      return std::string(str);
    }
  private:
    uint64_t _idValue;
  };

  class StateANDSDate_key
    : public EntityKey<key_traits<StateANDSDate_key> > {
  public:
    StateANDSDate_key(uint8_t state, time_t sendDate)
      : _state(state), _sendDate(sendDate) {}
    // It's constructor for making InfoSme_T_StateANDSDate_key object from data of
    // InfoSme_T object.
    StateANDSDate_key(const InfoSme_T_Entity& dataValue)
      : _state(dataValue.getState()), _sendDate(dataValue.getSendDate()) {}

    virtual bool operator== (const StateANDSDate_key& rhs) const {
      if ( _state == rhs._state && 
           _sendDate == rhs._sendDate) return true;
      else return false;
    }
    uint8_t getState() const { return _state; }
    time_t getSendDate() const { return _sendDate; }
    virtual bool operator< (const StateANDSDate_key& rhs) const {
      if ( _state < rhs._state ) return true;
      else if ( _state == rhs._state &&
                _sendDate < rhs._sendDate ) return true;
      else return false;
      /*if ( _sendDate <= rhs._sendDate && 
           _state < rhs._state
           ) return true;*/
      /*if ( _state <= rhs._state && 
        _sendDate < rhs._sendDate ) return true;
        else return false;*/
    }

    virtual std::string toString() const {
      char str[64];
      snprintf(str,sizeof(str),"state=[%d],sendDate=[%d]",_state,_sendDate);
      return std::string(str);
    }
  private:
    uint8_t _state;
    time_t _sendDate;
  };

private:
  uint64_t    _id;
  uint8_t     _state;
  std::string _abonentAddress;
  time_t      _sendDate;
  std::string _message;
  std::string _regionId;
};


#endif
