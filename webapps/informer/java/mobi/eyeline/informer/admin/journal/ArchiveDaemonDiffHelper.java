package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class ArchiveDaemonDiffHelper extends DiffHelper{

  ArchiveDaemonDiffHelper() {
    super(Subject.ARCHIVE_DAEMON);
  }

  void logArchiveDaemonStart(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_START, subject, user, "archivedaemon_start");
  }

  void logArchiveDaemonStop(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_STOP, subject, user, "archivedaemon_stop");

  }

  void logArchiveDaemonSwitch(Journal j, String toHost, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_NODE_SWITCH, subject, user, "archivedaemon_switch", toHost);

  }
}
