package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smsc.Smsc;

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
class SmscDiffHelper extends DiffHelper{

  SmscDiffHelper() {
    super(Subject.SMSC);
  }

  void logImmediateErrors(Smsc oldSmsc, Smsc newSmsc, Journal journal, String userName) throws AdminException {
    for(Integer o : oldSmsc.getImmediateErrors()) {
      boolean exist = false;
      for(Integer n : newSmsc.getImmediateErrors()) {
        if(o.equals(n)) {
          exist = true;
          break;
        }
      }
      if(!exist) {
        journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "smsc_immediate_remove", Integer.toString(o), oldSmsc.getName());
      }
    }
    for(Integer n : newSmsc.getImmediateErrors()) {
      boolean exist = false;
      for(Integer o : oldSmsc.getImmediateErrors()) {
        if(n.equals(o)) {
          exist = true;
          break;
        }
      }
      if(!exist) {
        journal.addRecord(JournalRecord.Type.ADD, subject, userName, "smsc_immediate_add", Integer.toString(n), newSmsc.getName());
      }
    }
  }

  void logPermErrors(Smsc oldSmsc, Smsc newSmsc, Journal journal, String userName) throws AdminException {
    for(Integer o : oldSmsc.getPermanentErrors()) {
      boolean exist = false;
      for(Integer n : newSmsc.getPermanentErrors()) {
        if(o.equals(n)) {
          exist = true;
          break;
        }
      }
      if(!exist) {
        journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "smsc_permanent_remove", Integer.toString(o), oldSmsc.getName());
      }
    }

    for(Integer n : newSmsc.getPermanentErrors()) {
      boolean exist = false;
      for(Integer o : oldSmsc.getPermanentErrors()) {
        if(n.equals(o)) {
          exist = true;
          break;
        }
      }
      if(!exist) {
        journal.addRecord(JournalRecord.Type.ADD, subject, userName, "smsc_permanent_add", Integer.toString(n), newSmsc.getName());
      }
    }

  }

  void logTempErrors(Smsc oldSmsc, Smsc newSmsc, Journal journal, String userName) throws AdminException {
    for(Map.Entry<String, Collection<Integer>> oe : oldSmsc.getTemporaryErrors().entrySet()) {
      String o = oe.getKey();
      boolean exist = false;
      for(Map.Entry<String, Collection<Integer>> ne : newSmsc.getTemporaryErrors().entrySet()) {
        String n = ne.getKey();
        if(o.equals(n)) {
          exist = true;
          Collection<Integer> oi = oe.getValue();
          Collection<Integer> ni = ne.getValue();

          if(oi.size() != ni.size()) {
            journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "smsc_temporary_changed", o, oldSmsc.getName());
          }else {
            for(Integer oldInt : oi) {
              boolean removed = true;
              for(Integer newInt : ni) {
                if(oldInt.equals(newInt)) {
                  removed= false;
                  break;
                }
              }
              if(removed) {
                journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "smsc_temporary_changed", o, oldSmsc.getName());
                break;
              }
            }
          }
          break;
        }
      }
      if(!exist) {
        journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "smsc_temporary_remove", o, oldSmsc.getName());
      }
    }

    for(String n : newSmsc.getTemporaryErrors().keySet()) {
      boolean exist = false;
      for(String o : oldSmsc.getTemporaryErrors().keySet()) {
        if(n.equals(o)) {
          exist = true;
          break;
        }
      }
      if(!exist) {
        journal.addRecord(JournalRecord.Type.ADD, subject, userName, "smsc_temporary_add", n, newSmsc.getName());
      }
    }
  }

  void logChanges(Smsc oldSmsc, Smsc newSmsc, Journal journal, String userName) throws AdminException {
    List<Method> getters = getGetters(Smsc.class, "getImmediateErrors","getPermanentErrors","getTemporaryErrors");
    List<Object> oldValues = callGetters(getters, oldSmsc);
    List<Object> newValues = callGetters(getters, newSmsc);
    logChanges(journal, oldValues, newValues, getters, userName, "smsc_property_changed", oldSmsc.getName());

    logImmediateErrors(oldSmsc, newSmsc, journal, userName);

    logPermErrors(oldSmsc, newSmsc, journal, userName);

    logTempErrors(oldSmsc, newSmsc, journal, userName);

  }

  void logAddSmsc(String newSmsc, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.ADD, subject, userName, "smsc_added", newSmsc);
  }

  void logRemoveSmsc(String smscName, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.REMOVE, subject, userName, "smsc_removed", smscName);
  }

  void logSetDefault(String oldSmsc, String newSmsc, Journal journal, String userName) throws AdminException {
    journal.addRecord(JournalRecord.Type.CHANGE, subject, userName, "smsc_default_changed", oldSmsc, newSmsc );
  }

}
