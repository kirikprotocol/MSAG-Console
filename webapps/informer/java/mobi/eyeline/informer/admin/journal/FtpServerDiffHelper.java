package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * User: artem
 * Date: 26.01.11
 */
class FtpServerDiffHelper extends DiffHelper {

  public FtpServerDiffHelper() {
    super(Subject.FTPSERVER);
  }

  public void logFtpServerStart(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_START, subject, user, "ftpserver_start");
  }

  public void logFtpServerStop(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_STOP, subject, user, "ftpserver_stop");

  }

  public void logFtpServerSwitch(Journal j, String toHost, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_NODE_SWITCH, subject, user, "ftpserver_switch", toHost);

  }
}
