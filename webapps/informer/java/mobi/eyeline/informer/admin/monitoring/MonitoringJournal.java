package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public interface MonitoringJournal {

  /**
   * Добавление событий в журнал
   *
   * @param events события
   * @throws mobi.eyeline.informer.admin.AdminException ошибка при сохранении событий
   */
  void addEvents(MonitoringEvent... events) throws AdminException;

  /**
   * Просмотр событий, удовлетворяющих фильтру
   *
   * @param filter фильтр
   * @param v посетитель
   * @throws AdminException ошибка при просмторе журнала
   */
  void visit(MonitoringFilter filter, Visitor v) throws AdminException;


  public static interface Visitor {

    public boolean visit(MonitoringEvent e) throws AdminException;

  }
}
