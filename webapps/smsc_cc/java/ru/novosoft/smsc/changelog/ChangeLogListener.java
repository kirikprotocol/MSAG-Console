package ru.novosoft.smsc.changelog;

/**
 * Интерфейс, описывающий листенера ChangeLog-а.
 * @author Artem Snopkov
 */
public interface ChangeLogListener {

  public void propertyChanged(ChangeLog.Source source, String object, Class objectClass,  String propertyName, Object oldValue, Object newValue) ;

  public void objectAdded(ChangeLog.Source source, Object object) ;

  public void objectRemoved(ChangeLog.Source source, Object object) ;
}
