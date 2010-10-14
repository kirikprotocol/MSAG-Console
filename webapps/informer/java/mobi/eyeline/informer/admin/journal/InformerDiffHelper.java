package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class InformerDiffHelper extends DiffHelper{

  public InformerDiffHelper() {
    super(Subject.INFORMER);
  }

  public void logInformerStart(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_START, subject, user, "informer_start");
  }

  public void logInformerStop(Journal j, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_STOP, subject, user, "informer_stop");

  }

  public void logInformerSwitch(Journal j, String toHost, String user) throws AdminException {
    j.addRecord(JournalRecord.Type.SERVICE_NODE_SWITCH, subject, user, "informer_switch", toHost); 

  }
}
