/* ************************************************************************* *
 * SMS/USSD/xSMS messages billing modes definition.
 * ************************************************************************* */
#ifndef __INMAN_BILLING_MODES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_BILLING_MODES_HPP

namespace smsc    {
namespace inman   {

class BillModes {
public:
  enum MsgType_e  { msgUnknown = 0, msgSMS = 1, msgUSSD = 2, msgXSMS = 3 };
  enum BillType_e { billOFF = 0, bill2CDR = 1, bill2IN = 2 };

  //Billing modes processing priority setting:
  // primary billing mode and secondary, that is used in case
  // of failure while processing the first one
  struct ProcPrio {
    BillType_e first;
    BillType_e second;

    ProcPrio() : first(billOFF), second(billOFF)
    { }

    bool operator==(const ProcPrio & cmp_obj) const
    {
      return (first == cmp_obj.first) && (second == cmp_obj.second);
    }
    bool operator!=(const ProcPrio & cmp_obj) const
    {
      return !(*this == cmp_obj);
    }
  };

  static const char * msgTypeStr(MsgType_e msg_id) { return k_nmMSGtypes[msg_id]; }
  static const char * billModeStr(BillType_e mode_id) { return k_nmBILLmodes[mode_id]; }
  static MsgType_e    str2MsgType(const char * id_str);

  BillModes()
  { }
  //
  ~BillModes()
  { }

  //Returns true if modes were previously assigned.
  bool assign(MsgType_e msg_type, BillType_e mode_1st, BillType_e mode_2nd = billOFF);
  //Returns true if billing mode is set to billOFF for all types of messages.
  bool allOFF(void) const;
  //Returns true if billing mode is set to bill2IN for at least one type of messages.
  bool useIN(void) const;
  //
  bool isAssigned(MsgType_e msg_type) const { return m_bmMap[msg_type].m_isAssigned; }
  //
  const ProcPrio * modeFor(MsgType_e msg_type) const
  {
    return m_bmMap[msg_type].m_isAssigned ? &(m_bmMap[msg_type].m_bmPrio)
                                          : &(m_bmMap[msgUnknown].m_bmPrio);
  }

protected:
  struct ArrEntry {
    bool      m_isAssigned;
    ProcPrio  m_bmPrio;

    ArrEntry() : m_isAssigned(false)
    { }
  };

  static const char * const k_nmMSGtypes[];   // = {"unknown", "SMS", "USSD", "XSMS"};
  static const char * const k_nmBILLmodes[];  // = {"OFF", "CDR", "IN"};

  ArrEntry  m_bmMap[msgXSMS + 1];
};

typedef BillModes::MsgType_e    MSG_TYPE;
typedef BillModes::BillType_e   BILL_MODE;
typedef BillModes::ProcPrio     BillModesPrio;

struct TrafficBillModes {
  BillModes   mo; //Billing modes for MobileOriginating messages
  BillModes   mt; //Billing modes for MobileTerminating messages

  bool useIN(void) const { return mo.useIN() || mt.useIN(); }
};


} //inman
} //smsc
#endif /* __INMAN_BILLING_MODES_HPP */

