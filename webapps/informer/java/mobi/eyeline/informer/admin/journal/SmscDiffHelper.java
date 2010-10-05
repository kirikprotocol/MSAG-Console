package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;

import java.lang.reflect.Method;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class SmscDiffHelper extends DiffHelper{

  public SmscDiffHelper(Subject subject) {
    super(subject);
  }

  public void logChanges(Smsc oldSmsc, Smsc newSmsc, Journal journal, String userName) throws AdminException {
    List<Method> getters = getGetters(Smsc.class);
    List<Object> oldValues = callGetters(getters, oldSmsc);
    List<Object> newValues = callGetters(getters, newSmsc);
    logChanges(journal, oldValues, newValues, getters, userName, "smsc_property_changed", oldSmsc.getName());
  }

  public void logAddSmsc(String newSmsc, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.ADD, subject, userName, "smsc_added", newSmsc);
  }

  public void logRemoveSmsc(String smscName, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "smsc_removed", smscName);
  }

  public void logSetDefault(String oldSmsc, String newSmsc, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "smsc_default_changed", oldSmsc, newSmsc );
  }

}
