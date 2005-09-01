#ifndef __SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MESSAGE_HPP_

#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>

extern "C" {
#include <constr_TYPE.h>
#include <MessageType.h>
}

namespace smsc{namespace mtsmsme{namespace processor{

extern BIT_STRING_t tcapversion;
extern ASN__PRIMITIVE_TYPE_t pduoid;

using std::vector;
using std::string;
struct TrId {
  uint8_t buf[4];
  int size;
  bool operator==(const TrId& obj)const
  {
    return size == obj.size && memcmp(&buf[0],&obj.buf[0],sizeof(uint8_t)*size) == 0;
  }
  string toString()
  {
    char tmpstr[9];
    char* text = tmpstr;
    int k = 0;
    for ( int i=0; i<size; i++){
      k+=sprintf(text+k,"%02X",buf[i]);
    }
    text[k]=0;
    return string(text);
  }
};

struct AC {
  vector<unsigned long> arcs;

  AC(){}
  void init(unsigned long *_arcs, int _size)
  {
    arcs.clear();
    arcs.reserve(_size);
    arcs.insert(arcs.begin(),_arcs,_arcs+_size);
  }
  AC(unsigned long *_arcs, int _size)
  {
    arcs.reserve(_size);
    arcs.insert(arcs.begin(),_arcs,_arcs+_size);
  }
  //AC(const AC& ac){init(&ac.arcs[0],ac.arcs.size());}
  bool operator==(const AC& obj)const
  {
    return arcs.size()==obj.arcs.size() &&
           memcmp(&arcs[0],&obj.arcs[0],sizeof(unsigned long)*arcs.size())==0;
  }
  bool operator!=(const AC& obj)const
  {
    return arcs.size()!=obj.arcs.size() ||
           memcmp(&arcs[0],&obj.arcs[0],sizeof(unsigned long)*arcs.size())!=0;
  }
  string toString()
  {
    int k = 0;
    int len = arcs.size();
    char text[32] = {0,};
    std::vector<unsigned char> stream;

    for(int i = 0; i < len; )
    {
      k=sprintf(text,"%ld",arcs[i]);
      stream.insert(stream.end(),text,text+k);
      if(++i < len) stream.push_back('.');
    }
    string result((char*)&stream[0],(char*)&stream[0]+stream.size());
    return result;
  }
};
extern AC sm_mt_relay_v1;
extern AC null_ac;
class ContMsg {
  public:
    ContMsg()
    {
      cont.present = MessageType_PR_contiinue;
      cont.choice.contiinue.dialoguePortion = 0;
      cont.choice.contiinue.componenets = 0;
      dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.size=0;
      dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf=0;
    }
    ~ContMsg()
    {
      if(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf)
         free(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf);
    }
    void setOTID(TrId _otid)
    {
      memcpy(otid,_otid.buf,_otid.size);
      cont.choice.contiinue.otid.buf = otid;
      cont.choice.contiinue.otid.size = _otid.size;
    }

    void setDTID(TrId _dtid)
    {
      memcpy(dtid,_dtid.buf,_dtid.size);
      cont.choice.contiinue.dtid.buf = dtid;
      cont.choice.contiinue.dtid.size = _dtid.size;
    }
    void setDialog(AC& _ac)
    {
      if(_ac == sm_mt_relay_v1 || _ac == null_ac) return;
      ac = _ac;
      dp.direct_reference = &pduoid;
      dp.indirect_reference = 0;
      dp.data_value_descriptor = 0;
      dp.encoding.present = encoding_PR_single_ASN1_type;
      dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueResponse;
      AARE_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueResponse;
      r.protocol_version = &tcapversion;
      OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],sizeof(unsigned long),ac.arcs.size());
      r.result = Associate_result_accepted;
      r.result_source_diagnostic.present = Associate_source_diagnostic_PR_dialogue_service_user;
      r.result_source_diagnostic.choice.dialogue_service_user = dialogue_service_user_null;
      r.aare_user_information = 0;
      cont.choice.contiinue.dialoguePortion = ( DialoguePortion *)&dp;
    }

    void setComponent(int result, int iid)
    {
      if(result)
      {
        comp.present = Component_PR_returnError;
        comp.choice.returnError.invokeID = iid;
        comp.choice.returnError.errorCode.present = Error_PR_local;
        comp.choice.returnError.errorCode.choice.local = 31;
        comp.choice.returnError.parameter = 0;
      }
      else
      {
        comp.present = Component_PR_returnResultLast;
        comp.choice.returnResultLast.invokeID = iid;
        comp.choice.returnResultLast.result = 0;
      }
      arr[0]= &comp;
      comps.list.count = 1;
      comps.list.size = 1;
      comps.list.array = arr;
      cont.choice.contiinue.componenets = &comps;
    }

    MessageType_t cont;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    Error_t errcode;
    AC ac;
    uint8_t otid[4];
    uint8_t dtid[4];
};

class EndMsg {
  public:
    EndMsg()
    {
      end.present = MessageType_PR_end;
      end.choice.end.dialoguePortion = 0;
      end.choice.end.components = 0;
      dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.size=0;
      dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf=0;
    }
    ~EndMsg()
    {
      if(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf)
         free(dp.encoding.choice.single_ASN1_type.choice.dialogueResponse.application_context_name.buf);

    }
    void setTrId(TrId dtid)
    {
      memcpy(trid,dtid.buf,dtid.size);
      end.choice.end.dtid.buf = trid;
      end.choice.end.dtid.size = dtid.size;
    }
    void setDialog(AC& _ac)
    {
      if(_ac == sm_mt_relay_v1 || _ac == null_ac) return;
      ac = _ac;
      dp.direct_reference = &pduoid;
      dp.indirect_reference = 0;
      dp.data_value_descriptor = 0;
      dp.encoding.present = encoding_PR_single_ASN1_type;
      dp.encoding.choice.single_ASN1_type.present = DialoguePDU_PR_dialogueResponse;
      AARE_apdu_t& r = dp.encoding.choice.single_ASN1_type.choice.dialogueResponse;
      r.protocol_version = &tcapversion;
      OBJECT_IDENTIFIER_set_arcs(&r.application_context_name,&ac.arcs[0],sizeof(unsigned long),ac.arcs.size());
      r.result = Associate_result_accepted;
      r.result_source_diagnostic.present = Associate_source_diagnostic_PR_dialogue_service_user;
      r.result_source_diagnostic.choice.dialogue_service_user = dialogue_service_user_null;
      r.aare_user_information = 0;
      end.choice.end.dialoguePortion = ( DialoguePortion *)&dp;
    }
    void setComponent(int result, int iid)
    {
      if(result)
      {
        comp.present = Component_PR_returnError;
        comp.choice.returnError.invokeID = iid;
        comp.choice.returnError.errorCode.present = Error_PR_local;
        comp.choice.returnError.errorCode.choice.local = 31;
        comp.choice.returnError.parameter = 0;
      }
      else
      {
        comp.present = Component_PR_returnResultLast;
        comp.choice.returnResultLast.invokeID = iid;
        comp.choice.returnResultLast.result = 0;
      }
      arr[0]= &comp;
      comps.list.count = 1;
      comps.list.size = 1;
      comps.list.array = arr;
      end.choice.end.components = &comps;
    }

    MessageType_t end;
    EXT_t         dp;
    ComponentPortion_t comps;
    Component_t *arr[1];
    Component_t comp;
    Error_t errcode;
    AC ac;

    uint8_t trid[4];
};
class Message {
  public:
    TrId getOTID();
    TrId getDTID();
    bool isBegin();
    bool isContinue();
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
