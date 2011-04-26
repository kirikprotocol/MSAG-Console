package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class PvssDiffHelper extends DiffHelper{

  PvssDiffHelper() {
    super(Subject.PVSS);
  }

  public void logPvssStart(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_START, subject, user, "pvss_start");
  }

  public void logPvssStop(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_STOP, subject, user, "pvss_stop");

  }

  public void logPvssSwitch(Journal j, String toHost, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_NODE_SWITCH, subject, user, "pvss_switch", toHost);
  }

}
