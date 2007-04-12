#ident "$Id$"

#ifndef __SMSC_INMAN_USS_USSMESSAGES__
#define __SMSC_INMAN_USS_USSMESSAGES__

#include <logger/Logger.h>
#include "util/Factory.hpp"
using smsc::util::FactoryT;

#include "util/TonNpiAddress.hpp"
using smsc::util::TonNpiAddress;

#include "inman/interaction/serializer.hpp"
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObjectAC;
using smsc::inman::interaction::SerializerITF;

#include <sstream>

#include <util/BinDump.hpp>

//NOTE: USSman considers the USS request encoding with DCS == 0xF4 as plain LATIN1 text
#define USSMAN_LATIN1_DCS 0xF4 // 1111 0100

namespace smsc  {
namespace inman {
namespace interaction {

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
                    public FactoryT<unsigned short, SerializableObjectAC> {
public:
  virtual ~SerializerUSS() { }

  SerializablePacketAC * deserialize(ObjectBuffer & in) throw(SerializerException);
  SerializablePacketAC * deserialize(std::auto_ptr<ObjectBuffer>& p_in) throw(SerializerException);

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
class USSPacketAC : public SerializablePacketAC { //[0] = cmdObject
protected:
    uint32_t    dlgId;

public:
    USSPacketAC(uint32_t dlg_id = 0) : dlgId(dlg_id) { Resize(1); }
   
    SerializableObjectAC * pCmd(void) { return at(0); }
    uint32_t dialogId(void) const { return dlgId; }
    void setDialogId(uint32_t dlg_id) { dlgId = dlg_id; }

    //SerializablePacketAC interface implementation
    void serialize(ObjectBuffer& out_buf) throw(SerializerException)
    {
        out_buf << at(0)->Id();
        out_buf << dlgId;
        at(0)->save(out_buf);
    }
};

//Template class for solid packet construction.
template<class _Command /* : public USSMessageAC*/>
class USSSolidPacketT : public USSPacketAC {
protected:
    _Command pckCmd;

public:
    USSSolidPacketT() : USSPacketAC()
        { referObj(0, pckCmd); }
    //constructor for copying
    USSSolidPacketT(const USSSolidPacketT &org_pck) : *this(org_pck)
        { referObj(0, pckCmd); } //fix at(0) reference

    _Command & Cmd(void) { return pckCmd; }
};

// --------------------------------------------------------- //
// USSMan commands: 
// --------------------------------------------------------- // 
typedef std::vector<unsigned char> USSDATA_T;

//Abstract class for USSman commands
class USSMessageAC : public SerializableObjectAC {
public:
  USSMessageAC(unsigned short msgTag) : SerializableObjectAC(msgTag),
                                        _logger(smsc::logger::Logger::getInstance("smsc.ussbalance")),
                                        _dCS(0)
  {}
  virtual ~USSMessageAC() {}

  //SerializableObjectAC interface:
  virtual void load(ObjectBuffer &in) throw(SerializerException);
  virtual void save(ObjectBuffer &out) const;

  //assigns USS data, that is plain LATIN1 text,
  void setUSSData(const unsigned char * data, unsigned size);
  void setUSSData(const USSDATA_T& data);

  //assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit, etc)
  void setRAWUSSData(unsigned char dcs, const USSDATA_T& ussdata);

  void setMSISDNadr(const TonNpiAddress& msadr) { _msAdr = msadr; }
  void setMSISDNadr(const char * adrStr) throw (CustomException);

  const USSDATA_T& getUSSData(void) const { return _ussData; }
  const TonNpiAddress& getMSISDNadr(void) const { return _msAdr; }
  unsigned char    getDCS(void) const     { return _dCS; }

  virtual std::string toString() const {
    std::ostringstream obuf;
    obuf << "msAddr=[" <<_msAdr.toString()
         << "],dCS=[" << static_cast<uint32_t>(_dCS)
         << "],ussData=[" << smsc::util::DumpHex(_ussData.size(), &_ussData[0])
         << "]";
    return obuf.str();
  }
protected:
  unsigned char   _dCS;
  USSDATA_T       _ussData;
  TonNpiAddress   _msAdr;
  smsc::logger::Logger* _logger; // for debug
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

  virtual std::string toString() const {
    std::ostringstream obuf(USSMessageAC::toString());
    obuf << ",IN_SSN=[" << static_cast<uint32_t>(_inSSN)
         << "],INAddr=[" << _inAddr.toString()
         << "]";
    return obuf.str();
  }
private:
  unsigned char _inSSN;
  TonNpiAddress _inAddr;
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

  virtual std::string toString() const {
    std::ostringstream obuf;
    obuf << "status=[" << _status
         << "]";
    if ( !_status )
      obuf << "," 
           << USSMessageAC::toString();
    return obuf.str();
  }

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
public:
    virtual void onProcessUSSRequest(USSRequestMessage* req) = 0;
    virtual void onDenyUSSRequest(USSRequestMessage* req) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_USS_USSMESSAGES__ */
