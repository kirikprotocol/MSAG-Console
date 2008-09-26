/* ************************************************************************** *
 * Containers for SCCP Message indications coming from SUA layer.
 * ************************************************************************** */
#ifndef __TCAP_SUA_INDICATIONS_HPP
#ident "@(#)$Id$"
#define __TCAP_SUA_INDICATIONS_HPP

#include "sua/libsua/MessageInfo.hpp"
using libsua::MessageInfo;

namespace eyelinecom {
namespace tcap {

//SCCP message indication parser abstract class
class SCCPMessageIndAC {
public:
    SCCPMessageIndAC()
    { }
    virtual ~SCCPMessageIndAC()
    { }

    //parses MessageInfo::msgData buffer, sets references to
    //MessageInfo::msgData buffer
    virtual bool parseMsgInfo(const MessageInfo & use_buf) = 0;
};

class SCCPUnitdataInd : public SCCPMessageIndAC {
protected:
    enum { has_SEQUENCE_CONTROL = 0x01 };
    uint8_t _fieldsMask;
    uint32_t _sequenceControl;
    uint8_t _calledAddrLen;
    const uint8_t * _calledAddr;   //encoded SCCP address, refers to MessageInfo::msgData buffer
    uint8_t callingAddrLen;
    const uint8_t * _callingAddr;  //encoded SCCP address, refers to MessageInfo::msgData buffer
    uint16_t _userDataLen;
    const uint8_t * _userData;     //encoded user data, refers to MessageInfo::msgData buffer


public:
    SCCPUnitdataInd()
        : _fieldsMask(0), _sequenceControl(0)
        , _calledAddrLen(0), _calledAddr(0), callingAddrLen(0)
        , _callingAddr(0), _userDataLen(0), _userData(0)
    { }
    ~SCCPUnitdataInd()
    { }


    //parses MessageInfo::msgData buffer, sets references to
    //MessageInfo::msgData buffer
    virtual bool parseMsgInfo(const MessageInfo & use_buf);

    inline uint32_t sequenceControl(void) const
    {
        return (_fieldsMask & has_SEQUENCE_CONTROL) ? _sequenceControl : 0;
    }
    inline uint8_t calledAddrLen(void) const { return _calledAddrLen; }
    inline const uint8_t * calledAddr(void) const { return calledAddr; }
    inline uint8_t callingAddrLen(void) const { return _callingAddrLen; }
    inline const uint8_t * callingAddr(void) const { return callingAddr; }
    inline uint16_t userDataLen(void) const { return _userDataLen; }
    inline const uint8_t * userData(void) const { return _userData; }
};


class SCCPNoticeInd : public SCCPMessageIndAC {
protected:
    enum { has_RETURN_REASON = 0x01, has_IMPORTANCE = 0x02 };
    uint8_t _fieldsMask;

    uint8_t _returnReason;
    uint8_t _importance;

    uint8_t _calledAddrLen;
    const uint8_t * _calledAddr;   //encoded SCCP address, refers to MessageInfo::msgData buffer
    uint8_t callingAddrLen;
    const uint8_t * _callingAddr;  //encoded SCCP address, refers to MessageInfo::msgData buffer
    uint16_t _userDataLen;
    const uint8_t * _userData;     //enocoded message, refers to MessageInfo::msgData buffer

public:
    SCCPNoticeInd()
        : _fieldsMask(0), _returnReason(0), _importance(0) 
        , _calledAddrLen(0), _calledAddr(0), callingAddrLen(0)
        , _callingAddr(0), _userDataLen(0), _userData(0)
    { }
    ~SCCPNoticeInd()
    { }


    //parses MessageInfo::msgData buffer, sets references to
    //MessageInfo::msgData buffer
    virtual bool parseMsgInfo(const MessageInfo & use_buf);


    inline uint32_t returnReason(void) const
    {
        return (_fieldsMask & has_RETURN_REASON) ? _returnReason : 0;
    }
    inline uint32_t sequenceControl(void) const
    {
        return (_fieldsMask & has_IMPORTANCE) ? _importance : 0;
    }

    inline uint8_t calledAddrLen(void) const { return _calledAddrLen; }
    inline const uint8_t * calledAddr(void) const { return calledAddr; }
    inline uint8_t callingAddrLen(void) const { return _callingAddrLen; }
    inline const uint8_t * callingAddr(void) const { return callingAddr; }
    inline uint16_t userDataLen(void) const { return _userDataLen; }
    inline const uint8_t * userData(void) const { return _userData; }
};


class SUAMessageInd : public MessageInfo {
public:
    union {
        const SCCPUnitdataInd * udt;
        const SCCPNoticeInd   * ntc;
        SCCPMessageIndAC * ind;
    } u;

    SCCPMessageIndAC()
    {
        u.ind = 0;
    }
    ~SCCPMessageIndAC()
    {
        reset();
    }

    inline void reset(void)
    {
        if (u.ind) {
            delete u.ind;
            u.ind = 0;
        }
    }
    //parses MessageInfo::msgData buffer
    inline bool parseMsgData(void) 
    {   if (u.ind)
            return true;
        if (messageType == libsua::N_UNITDATA_IND_MSGCODE)
            u.udt = new SCCPUnitdataInd();
        else if (messageType == libsua::N_NOTICE_IND_MSGCODE)
            u.ntc = new SCCPNoticeInd();
        }
        return u.ind->parseMsgInfo(this);
    }
};


}; //tcap
}; //eyelinecom

#endif /* __TCAP_SUA_INDICATIONS_HPP */

