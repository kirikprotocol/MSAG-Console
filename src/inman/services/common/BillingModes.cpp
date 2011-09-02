#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <string.h>
#include "inman/services/common/BillingModes.hpp"

namespace smsc {
namespace inman {

const char * const BillModes::k_nmBILLmodes[] = {"OFF", "CDR", "IN"};
const char * const BillModes::k_nmMSGtypes[] = {"unknown", "SMS", "USSD", "XSMS"};

BillModes::MsgType_e BillModes::str2MsgType(const char * id_str)
{
  if (!strcmp(BillModes::msgTypeStr(msgSMS), id_str))
    return msgSMS;
  if (!strcmp(BillModes::msgTypeStr(msgUSSD), id_str))
    return msgUSSD;
  if (!strcmp(BillModes::msgTypeStr(msgXSMS), id_str))
    return msgXSMS;
  return msgUnknown;
}

//Returns true if modes were previously assigned.
bool BillModes::assign(BillModes::MsgType_e msg_type, BillModes::BillType_e mode_1st,
                       BillModes::BillType_e mode_2nd/* = BillParms::billOFF*/)
{
  bool rval = m_bmMap[msg_type].m_isAssigned;
  m_bmMap[msg_type].m_isAssigned = true;
  m_bmMap[msg_type].m_bmPrio.first = mode_1st;
  m_bmMap[msg_type].m_bmPrio.second = mode_2nd;
  return rval;
}

//Returns true if billing mode is set to billOFF for all types of messages.
bool BillModes::allOFF(void) const
{
  for (unsigned short i = msgUnknown + 1; i <= msgXSMS; ++i) {
    if (m_bmMap[i].m_isAssigned && (m_bmMap[i].m_bmPrio.first != billOFF))
      return false;
  }
  return true;
}

//Returns true if billing mode is set to bill2IN for at least one type of messages.
bool BillModes::useIN(void) const
{
  for (unsigned short i = msgUnknown + 1; i <= msgXSMS; ++i) {
    if (m_bmMap[i].m_isAssigned && (m_bmMap[i].m_bmPrio.first == bill2IN))
      return true;
  }
  return false;
}

} //inman
} //smsc

