#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_

#include <asn_application.h>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include "mtsmsme/processor/ACRepo.hpp"
#include "logger/Logger.h"
#include "TCMessage.h"

namespace smsc{namespace mtsmsme{namespace processor{

using smsc::logger::Logger;
using std::vector;
using std::string;

class BeginMsg {
  public:
    BeginMsg();
    ~BeginMsg();
    void setOTID(TrId _otid);
    void setDialog(AC& _ac);
    void setInvokeReq(int iid, uint8_t opcode, vector<unsigned char>& argument);
    void encode(vector<unsigned char>& buf);

    TCMessage_t begin;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    ANY_t argument;
    AC ac;
    uint8_t otid[4];
};
class ContMsg {
  public:
    ContMsg();
    ~ContMsg();
    void setOTID(TrId _otid);
    void setDTID(TrId _dtid);
    void setDialog(AC& _ac);
    void setInvokeReq(int iid, uint8_t opcode, vector<unsigned char>& argument);
    void setComponent(int result, int iid);
    void encode(vector<unsigned char>& buf);

    TCMessage_t cont;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    ANY_t argument;
    AC ac;
    uint8_t otid[4];
    uint8_t dtid[4];
  private:
    Logger* logger;
};

class EndMsg {
  public:
    EndMsg();
    EndMsg(Logger *logger);
    ~EndMsg();
    void setTrId(TrId dtid);
    void setDialog(AC& _ac);
    void setComponent(int result, int iid);
    void setReturnResultL(int iid, uint8_t opcode, vector<unsigned char>& argument);
    void setError(int iid, uint8_t errcode, vector<unsigned char>& argument);
    void encode(vector<unsigned char>& buf);

    TCMessage_t end;
    EXT_t        dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    struct ReturnResult::result res;
    AC ac;

    uint8_t trid[4];
  private:
    void fillComponentList();
    Logger* logger;
};
class Message {
  public:
    TrId getOTID();
    TrId getDTID();
    bool isBegin();
    bool isContinue();
    bool isEnd();
    bool isAbort();
    bool isComponentPresent();
    int getInvokeId();
    int getOperationCode();
    bool isDialoguePortionExist();
    void getAppContext(AC& ac);
    string toString();
    vector<unsigned char> getComponent();
    Message(Logger *logger);
    void decode(void *buf, int buflen);
    ~Message();
  private:
    void *structure;
    Logger* logger;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/

#endif //__SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_
