#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/common/BillModesCfgReader.hpp"
using smsc::util::config::XConfigView;

#include "util/csv/CSVArrayOf.hpp"
using smsc::util::csv::CSVArrayOfStr;

namespace smsc {
namespace inman {
/* ************************************************************************* *
 * class BillModesCfgReader implementation
 * ************************************************************************* */

//NOTE: 'root_sec' is a Config associated with a file containing BillingModes section
void BillModesCfgReader::parseSection(const Config & root_sec, const char * nm_sub_sec,
                                      TrafficBillModes & st_cfg) const
  throw(ConfigException)
{
  if (!root_sec.findSection(mNmSec))
    throw ConfigException("section %s' is missing!", mNmSec);

  XConfigView bmSec(root_sec, mNmSec);
  if (!bmSec.findSubSection(nm_sub_sec))
    throw ConfigException("section %s.%s' is missing!", mNmSec, nm_sub_sec);

  XConfigView subSec;
  bmSec.getSubConfig(subSec, nm_sub_sec);
  readBillingModes(subSec, st_cfg);
}


/* Reads BillingModes subsection */
void BillModesCfgReader::readBillingModes(const XConfigView & sub_cfg, TrafficBillModes & st_cfg) const
  throw(ConfigException)
{
  std::auto_ptr<CStrSet>  msgs(sub_cfg.getShortSectionNames());
  if (msgs->empty())
    throw ConfigException("no billing modes set");

  for (CStrSet::iterator sit = msgs->begin(); sit != msgs->end(); ++sit) {
    MSG_TYPE msgType = BillModes::str2MsgType(sit->c_str());
    
    if (msgType == BillModes::msgUnknown)
      throw ConfigException("Illegal section for messageType %s", sit->c_str());
    if (st_cfg.mo.isAssigned(msgType) || st_cfg.mt.isAssigned(msgType))
      throw ConfigException("duplicate section for messageType %s", sit->c_str());

    XConfigView curMsg;
    sub_cfg.getSubConfig(curMsg, sit->c_str());
    readModesFor(msgType, curMsg, st_cfg);
  }
}

void BillModesCfgReader::readModesFor(MSG_TYPE msg_type, const XConfigView & msg_cfg,
                                      TrafficBillModes & st_cfg) const
  throw(ConfigException)
{
  const char * cstr = NULL;
  smsc_log_info(mLogger, "  %s ..", BillModes::msgTypeStr(msg_type));

  try { cstr = msg_cfg.getString("MO"); }
  catch (const ConfigException& exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("%s parameter 'MO' is invalid or missing!",
                              BillModes::msgTypeStr(msg_type));
  readBillMode(msg_type, cstr, st_cfg.mo);
  smsc_log_info(mLogger, "    MO -> %s, %s", 
                BillModes::billModeStr(st_cfg.mo.modeFor(msg_type)->first),
                BillModes::billModeStr(st_cfg.mo.modeFor(msg_type)->second));

  try { cstr = msg_cfg.getString("MT"); }
  catch (const ConfigException& exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("%s parameter 'MT' is invalid or missing!",
                              BillModes::msgTypeStr(msg_type));
  readBillMode(msg_type, cstr, st_cfg.mt);
  smsc_log_info(mLogger, "    MT -> %s, %s", 
                BillModes::billModeStr(st_cfg.mt.modeFor(msg_type)->first),
                BillModes::billModeStr(st_cfg.mt.modeFor(msg_type)->second));
}

void BillModesCfgReader::readBillMode(MSG_TYPE msg_type, const char * mode_str,
                                      BillModes & bm_cfg) const
  throw(ConfigException)
{
  BILL_MODE   pbm[2];
  str2BillMode(mode_str, pbm);

  //according to #B2501:
  if ((msg_type == BillModes::msgSMS) && (pbm[0] == BillModes::bill2IN)) {
    smsc_log_warn(mLogger, "Unsafe billMode '%s' for messageType '%s'",
                  BillModes::billModeStr(BillModes::bill2IN), BillModes::msgTypeStr(BillModes::msgSMS));
    smsc_log_warn(mLogger, "  consider double CDR creation for prepaid abonents by serving IN-point");
/*    throw ConfigException("Forbidden billMode '%s' for messageType '%s'",
                            BillModes::billModeStr(bill2IN), BillModes::msgTypeStr(msgSMS));*/
  }
  bm_cfg.assign(msg_type, pbm[0], pbm[1]);
}


void BillModesCfgReader::str2BillMode(const char * mode_str, BILL_MODE (& bm_prio)[2])
  throw(ConfigException)
{
  if (!mode_str || !mode_str[0])
    throw ConfigException("Invalid billMode");

  CSVArrayOfStr  bmList((CSVArrayOfStr::size_type)2);
  if (!bmList.fromStr(mode_str) || (bmList.size() > 2))
    throw ConfigException("Invalid billMode '%s'", mode_str);

  bm_prio[0] = bm_prio[1] = BillModes::billOFF;
  CSVArrayOfStr::size_type i = 0;
  do {
    if (!strcmp(BillModes::billModeStr(BillModes::bill2IN), bmList[i].c_str()))
      bm_prio[i] = BillModes::bill2IN;
    else if (!strcmp(BillModes::billModeStr(BillModes::bill2CDR), bmList[i].c_str())) {
      bm_prio[i] = BillModes::bill2CDR; // no need to check next value
      bm_prio[++i] = BillModes::bill2CDR; 
    } else if (!strcmp(BillModes::billModeStr(BillModes::billOFF), bmList[i].c_str()))
      ++i; // no need to check next value
    else
      throw ConfigException("Invalid billMode '%s'", bmList[i].c_str());
  } while (++i < bmList.size());

  //check bill2IN setting ..
  if ((bm_prio[0] == BillModes::bill2IN) && (bm_prio[0] == bm_prio[1]))
    throw ConfigException("Invalid billMode '%s'", mode_str);
}


} //inman
} //smsc

