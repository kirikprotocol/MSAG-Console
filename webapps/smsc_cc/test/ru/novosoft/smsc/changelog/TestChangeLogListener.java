package ru.novosoft.smsc.changelog;

/**
 * @author Artem Snopkov
 */
public class TestChangeLogListener implements ChangeLogListener {

  public int changed_calls;
  public ChangeLog.Subject changed_subject;
  public String changed_subjectDesc;
  public String changed_propertyName;
  public Object changed_oldValue;
  public Object changed_newValue;

  public int added_calls;
  public ChangeLog.Subject added_subject;
  public Object added_object;

  public int removed_calls;
  public ChangeLog.Subject removed_subject;
  public Object removed_object;

  public int apply_calls;
  public ChangeLog.Subject apply_subject;

  public int reset_calls;
  public ChangeLog.Subject reset_subject;

  public void propertyChanged(ChangeLog.Subject subject, String subjectDesc, Class subjectClass, String propertyName, Object oldValue, Object newValue) {
    changed_calls++;
    this.changed_subject = subject;
    this.changed_subjectDesc = subjectDesc;
    this.changed_propertyName = propertyName;
    this.changed_oldValue = oldValue;
    this.changed_newValue = newValue;
  }

  public void objectAdded(ChangeLog.Subject subject, Object object) {
    added_calls++;
    added_object = object;
    added_subject = subject;
  }

  public void objectRemoved(ChangeLog.Subject subject, Object object) {
    removed_calls++;
    removed_object = object;
    removed_subject = subject;
  }

  public void applyCalled(ChangeLog.Subject subject) {
    apply_calls++;
    this.apply_subject = subject;
  }

  public void resetCalled(ChangeLog.Subject subject) {
    reset_calls++;
    this.reset_subject = subject;
  }
}
