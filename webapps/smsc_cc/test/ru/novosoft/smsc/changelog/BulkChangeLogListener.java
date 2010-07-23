package ru.novosoft.smsc.changelog;

/**
 * @author Artem Snopkov
 */
public class BulkChangeLogListener implements ChangeLogListener {
  public void propertyChanged(ChangeLog.Subject subject, String subjectDesc, Class subjectClass, String propertyName, Object oldValue, Object newValue) {
  }

  public void objectAdded(ChangeLog.Subject subject, Object object) {
  }

  public void objectRemoved(ChangeLog.Subject subject, Object object) {
  }

  public void applyCalled(ChangeLog.Subject sublect) {
  }

  public void resetCalled(ChangeLog.Subject sublect) {
  }
}
