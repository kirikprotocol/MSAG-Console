package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.restriction.Restriction;

import java.lang.reflect.Method;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 08.11.2010
 * Time: 13:20:33
 */
public class RestrictionsDiffHelper extends DiffHelper {
  public RestrictionsDiffHelper() {
    super(Subject.RESTRICTIONS);
  }

  public void logRestrictionAdd(Journal journal, String user, Restriction r) throws AdminException {
    journal.addRecord(JournalRecord.Type.ADD, subject, user, "restriction_added", r.getName());
  }

  public void logUpdateRestriction(Journal j, String user, Restriction r, Restriction oldR) throws AdminException {
    List<Method> getters = getGetters(Restriction.class);
    List<Object> oldValues = callGetters(getters, oldR);
    List<Object> newValues = callGetters(getters, r);
    logChanges(j, oldValues, newValues, getters, user);
  }

  public void logDeleteRestriction(Journal journal, String user, Restriction r) throws AdminException {
    journal.addRecord(JournalRecord.Type.REMOVE, subject, user,"restriction_removed",r.getName(),r.getId()+"");
  }
}
