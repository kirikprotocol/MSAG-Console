package ru.novosoft.smsc.changelog;

/**
 * Интерфейс, описывающий листенера ChangeLog-а.
 * @author Artem Snopkov
 */
public interface ChangeLogListener {

  void propertyChanged(ChangeLog.Subject subject, String subjectDesc, Class subjectClass,  String propertyName, Object oldValue, Object newValue) ;

  void objectAdded(ChangeLog.Subject subject, Object object) ;

  void objectRemoved(ChangeLog.Subject subject, Object object) ;

  void applyCalled(ChangeLog.Subject sublect);

  void resetCalled(ChangeLog.Subject sublect);
}
