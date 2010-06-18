#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/xcfg/TCProvdCfgReader.hpp"
#include "eyeline/ss7na/libsccp/xcfg/LibSccpCfgReader.hpp"

#include "util/csv/CSValueTraitsInt.hpp"
#include "util/csv/CSVArrayOf.hpp"
#include "util/TonNpiAddress.hpp"

namespace eyeline  {
namespace tcap {

using smsc::util::config::XConfigView;

typedef smsc::util::csv::CSVArrayOf_T<uint8_t>  XSSNsArray;

/* ************************************************************************* *
 * class TCProvdCfgReader implementation
 * ************************************************************************* */
void TCProvdCfgReader::readConfig(Config & use_xcfg, TCProviderCfg & st_cfg)
    throw(ConfigException)
{
  if (!use_xcfg.findSection(_nmSec))
    throw ConfigException("'%s' section is missing", _nmSec);

  XConfigView cfgSec(use_xcfg, _nmSec);
  int32_t tInt = 0;
  const char * cstr = 0;

  //<param name="ownGT" type="string">
  cstr = 0;
  try { cstr = cfgSec.getString("ownGT"); }
  catch (...) { cstr = 0; }
  smsc::util::TonNpiAddress ownGT;
  if (!ownGT.fromText(cstr) || !ownGT.fixISDN())
    throw ConfigException("'%s.ownGT' parameter is invalid", cfgSec.relSection());
  st_cfg._tcapCfg._ownGT.construct(0, sccp::GlobalTitle::npiISDNTele_e164,
                          sccp::GlobalTitle::noaInternational, ownGT.getSignals());

  //<param name="subsystems" type="string">
  cstr = 0;
  try { cstr = cfgSec.getString("subSystems"); }
  catch (...) { cstr = 0; }

  if (cstr) {
    XSSNsArray  subSNs((XSSNsArray::size_type)8);
    try {
      if (!subSNs.fromStr(cstr))
        cstr = 0;
    } catch (const std::exception & ex) {
      throw ConfigException("'%s.subSystems' parameter is invalid (%s)",
                            cfgSec.relSection(), ex.what());
    }
    st_cfg._tcapCfg._subSNs = subSNs;
  }
  if (!cstr)
    throw ConfigException("'%s.subSystems' parameter is invalid or missing",
                          cfgSec.relSection());

  //<param name="initMsgReaders" type="int">
  tInt = 0;
  try { tInt = cfgSec.getInt("initMsgReaders"); }
  catch (...) { tInt = 0; }
  if (!tInt || (tInt < 0))
    throw ConfigException("'%s.initMsgReaders' parameter is invalid or missing", cfgSec.relSection());
  st_cfg._tcapCfg._iniMsgReaders = (unsigned)tInt;

  //<param name="maxMsgReaders" type="int">
  tInt = 0;
  try { tInt = cfgSec.getInt("maxMsgReaders"); }
  catch (...) { tInt = 0; }
  if (!tInt || (tInt < 0))
    throw ConfigException("'%s.maxMsgReaders' parameter is invalid or missing", cfgSec.relSection());
  st_cfg._tcapCfg._maxMsgReaders = (unsigned)tInt;

  //<param name="maxDialoguesCount" type="int"> 
  tInt = 0;
  try { tInt = cfgSec.getInt("maxDialoguesCount"); }
  catch (...) { tInt = 0; }
  if (!tInt || (tInt < 0))
    throw ConfigException("'%s.maxDialoguesCount' parameter is invalid or missing", cfgSec.relSection());
  st_cfg._tcapCfg._maxDlgNum = (unsigned)tInt;

  //<param name="defaultDlgTimeout" type="int"> uint16_t, default: 300
  tInt = 0;
  try { tInt = cfgSec.getInt("defaultDlgTimeout"); }
  catch (...) { tInt = TCProviderParms::_DFLT_DLG_TMO; }
  if (!tInt || (tInt < 0) || (tInt > 0xFFFF))
    throw ConfigException("'%s.maxDialoguesCount' parameter is invalid or missing", cfgSec.relSection());
  st_cfg._tcapCfg._dfltDlgTmo = (TDlgTimeout)tInt;

  //<param name="defaultRosTimeout" type="int"> uint16_t, default: 60
  tInt = 0;
  try { tInt = cfgSec.getInt("defaultRosTimeout"); }
  catch (...) { tInt = TCProviderParms::_DFLT_ROS_TMO; }
  if (!tInt || (tInt < 0) || (tInt > 0xFFFF))
    throw ConfigException("'%s.defaultRosTimeout' parameter is invalid or missing", cfgSec.relSection());
  st_cfg._tcapCfg._dfltROSTmo = (TDlgTimeout)tInt;

  //<param name="trnIdSelectionAlg" type="string">
  //TODO: 

  //parse sccp_provider configuration
  ss7na::libsccp::LibSccpCfgReader  sccpRdr;
  sccpRdr.readConfig(&cfgSec, st_cfg._sccpCfg);
  /**/
  return;
}


} //tcap
} //eyeline


