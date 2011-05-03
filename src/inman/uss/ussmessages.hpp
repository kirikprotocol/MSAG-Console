/* ************************************************************************** *
 * USSMan USS request execution protocol PDUs definition.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_USS_USSMESSAGES__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_USS_USSMESSAGES__

#include <logger/Logger.h>
#include "util/Factory.hpp"
#include "util/TonNpiAddress.hpp"

#include "inman/interaction/serializer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::util::TonNpiAddress;
using smsc::util::CustomException;

using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObjectAC;
using smsc::inman::interaction::SerializerITF;


// 1. USSRequest         ( client --> USSMAN )
// 2. USSRequestResult   ( client <-- USSMAN )

struct USS2CMD {
    enum _TAG {
        PROCESS_USS_REQUEST_TAG = 1,
        PROCESS_USS_RESULT_TAG = 2
    };
    enum _STATUS {
        STATUS_USS_REQUEST_OK = 0,      //request successfully processed
        STATUS_USS_REQUEST_DENIAL = 1,  //request is already in process
        STATUS_USS_REQUEST_FAILED = 2   //request failed because of USSMan encounter an error
    };
};

//DeSerializer for USSman commands, transferred over TCP socket
//NOTE: SerializerUSS doesn't provide partial deserialization of packets
class SerializerUSS : public SerializerITF, 
                    public smsc::util::FactoryT<unsigned short, SerializableObjectAC> {
public:
  virtual ~SerializerUSS() { }

  SerializablePacketAC * deserialize(ObjectBuffer & in) const throw(SerializerException);
  SerializablePacketAC * deserialize(std::auto_ptr<ObjectBuffer>& p_in) const throw(SerializerException);

  static SerializerUSS* getInstance();

protected:
  SerializerUSS();
  smsc::logger::Logger* _logger;
};

/*
 * USSMan messages are transferred as length prefixed packet
 * consisting of single serialized object(command) and have
 * the following serialization format:

  2b        4b              up to ...b
-------  ---------   -------------------------------
 msgId : requestId  :  message data                |
       |                                           |
           -- processed by load()/save() method --
*/
class USSPacketAC : public SerializablePacket_T<1> { //[0] = cmdObject
protected:
    uint32_t    dlgId;

public:
    explicit USSPacketAC(uint32_t dlg_id = 0)
      : SerializablePacket_T<1>(), dlgId(dlg_id)
    { }
    virtual ~USSPacketAC()
    { }
   
    SerializableObjectAC * const pCmd(void) { return getObj(0); }

    uint32_t dialogId(void) const { return dlgId; }
    void setDialogId(uint32_t dlg_id) { dlgId = dlg_id; }

    //SerializablePacketAC interface implementation
    void serialize(ObjectBuffer& out_buf) const throw(SerializerException)
    {
        out_buf << getObj(0)->Id();
        out_buf << dlgId;
        getObj(0)->save(out_buf);
    }
};

//Template class for solid packet construction.
template<class _Command /* : public USSMessageAC*/>
class USSSolidPacketT : public USSPacketAC {
protected:
    _Command pckCmd;

public:
    USSSolidPacketT() : USSPacketAC()
    {
      referObj(0, pckCmd);  //fix at(0) reference
    }
    //constructor for copying
    USSSolidPacketT(const USSSolidPacketT &org_pck) : USSPacketAC(org_pck)
    {
      referObj(0, pckCmd);  //fix at(0) reference
    }
    ~USSSolidPacketT()
    { }

    _Command & Cmd(void) { return pckCmd; }
    const _Command & Cmd() const { return pckCmd; }
};

// --------------------------------------------------------- //
// USSMan commands: 
// --------------------------------------------------------- // 
typedef std::vector<unsigned char> USSDATA_T;

//Abstract class for USSman commands
class USSMessageAC : public SerializableObjectAC {
protected:
  unsigned char   _dCS, _flg;
  bool            _dCS_wasRead;
  std::string     _latin1Text;
  USSDATA_T       _ussData;
  TonNpiAddress   _msAdr;
  smsc::logger::Logger* _logger; // for debug


  explicit USSMessageAC(unsigned short msgTag)
    : SerializableObjectAC(msgTag)
    , _dCS(0), _flg(UNKNOWN_CODING), _dCS_wasRead(false)
    , _logger(smsc::logger::Logger::getInstance("smsc.ussman"))
  { }

public:
  enum  DataFlag_e {
    PREPARED_USS_REQ=0, LATIN1_USS_TEXT, UCS2_USS_TEXT, UNKNOWN_CODING=0XFF
  };

  virtual ~USSMessageAC()
  { }

  //SerializableObjectAC interface:
  virtual void load(ObjectBuffer &in) throw(SerializerException);
  virtual void save(ObjectBuffer &out) const;

  //assigns USS data, that is plain LATIN1 text,
  void setUSSData(const char * data, unsigned size);

  //assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit, etc)
  void setRAWUSSData(unsigned char dcs, const USSDATA_T& ussdata);

  // assings UCS2 data
  void setUCS2USSData(const std::vector<uint8_t>& ucs2);

  void setMSISDNadr(const TonNpiAddress& msadr) { _msAdr = msadr; }
  void setMSISDNadr(const char * adrStr) throw (CustomException);

  unsigned char getFlg() const { return _flg; }
  const USSDATA_T& getUSSData(void) const { return _ussData; }
  const char* getLatin1Text() const { return _latin1Text.c_str(); }

  const TonNpiAddress& getMSISDNadr(void) const { return _msAdr; }
  unsigned char    getDCS(void) const     { return _dCS; }

  virtual std::string toString() const;
};


class USSRequestMessage : public USSMessageAC {
public:
  USSRequestMessage() : USSMessageAC(USS2CMD::PROCESS_USS_REQUEST_TAG) {}
  ~USSRequestMessage() {}

  //SerializableObjectAC interface:
  virtual void load(ObjectBuffer &in) throw(SerializerException);
  virtual void save(ObjectBuffer &out) const;

  void set_IN_ISDNaddr(const TonNpiAddress& inaddr) { _inAddr = inaddr; }
  void set_IN_ISDNaddr(const char * addrStr) throw (CustomException);
  void set_IN_SSN(unsigned char inSSN) { _inSSN = inSSN; }

  const TonNpiAddress& get_IN_ISDNaddr(void) const { return _inAddr; }
  unsigned char    get_IN_SSN(void) const     { return _inSSN; }

  const std::string& getIMSI() const { return _imsi; }

  virtual std::string toString() const;
private:
  unsigned char _inSSN;
  TonNpiAddress _inAddr;
  std::string _imsi;
};


class USSResultMessage : public USSMessageAC {
public:
  USSResultMessage() : USSMessageAC(USS2CMD::PROCESS_USS_RESULT_TAG) {}
  ~USSResultMessage() {};

  virtual void load(ObjectBuffer &in) throw(SerializerException);
  virtual void save(ObjectBuffer &out) const;

  bool  getUSSDataAsLatin1Text(std::string & str);
  unsigned short   getStatus(void) const  { return _status; }
  void setStatus(const unsigned short& status) { _status = status; }

  virtual std::string toString() const;

private:
  unsigned short  _status;
};

// --------------------------------------------------------- //
// Solid instances of USSMan packets:
// --------------------------------------------------------- //
typedef USSSolidPacketT<USSRequestMessage>  SPckUSSRequest;
typedef USSSolidPacketT<USSResultMessage>   SPckUSSResult;

// --------------------------------------------------------- //
// USSman command handler interface:
// --------------------------------------------------------- //
class USSCommandHandlerITF {
protected:
    virtual ~USSCommandHandlerITF() //forbid interface destruction
    { }

public:
    virtual void onProcessUSSRequest(USSRequestMessage* req) = 0;
    virtual void onDenyUSSRequest(USSRequestMessage* req) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_USS_USSMESSAGES__ */
