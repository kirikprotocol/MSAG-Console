package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWDiffHelper extends DiffHelper{

  SmppGWDiffHelper() {
    super(Subject.SMPPGW);
  }

  public void logSmppGWStart(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_START, subject, user, "smpp_gw_start");
  }

  public void logSmppGWStop(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_STOP, subject, user, "smpp_gw_stop");

  }

  public void logSmppGWSwitch(Journal j, String toHost, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_NODE_SWITCH, subject, user, "smpp_gw_switch", toHost);
  }

  public void logUpdateSmppGWProviders(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.CHANGE, subject, user, "smpp_gw_providers_update");
  }

  public void logUpdateSmppGWEndpoints(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.CHANGE, subject, user, "smpp_gw_endpoints_update");
  }
}
