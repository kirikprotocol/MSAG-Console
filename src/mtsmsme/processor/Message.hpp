#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_

#include <asn_application.h>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include "mtsmsme/processor/ACRepo.hpp"
extern "C" {
#include <constr_TYPE.h>
#include <TCMessage.h>
}

namespace smsc{namespace mtsmsme{namespace processor{

using std::vector;
using std::string;

class ContMsg {
  public:
    ContMsg();
    ~ContMsg();
    void setOTID(TrId _otid);
    void setDTID(TrId _dtid);
    void setDialog(AC& _ac);
    void setComponent(int result, int iid);

    TCMessage_t cont;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    AC ac;
    uint8_t otid[4];
    uint8_t dtid[4];
};

class EndMsg {
  public:
    EndMsg();
    ~EndMsg();
    void setTrId(TrId dtid);
    void setDialog(AC& _ac);
    void setComponent(int result, int iid);

    TCMessage_t end;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    AC ac;

    uint8_t trid[4];
};
class Message {
  public:
    TrId getOTID();
    TrId getDTID();
    bool isBegin();
    bool isContinue();
    bool isEnd();
    bool isComponentPresent();
    int getInvokeId();
    bool isDialoguePortionExist();
    void getAppContext(AC& ac);
    string toString();
    vector<unsigned char> getComponent();
    Message(void *structure);
    Message();
    void setStructure(void *structure);
    ~Message();
  private:
    void *structure;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/

#endif //__SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_
