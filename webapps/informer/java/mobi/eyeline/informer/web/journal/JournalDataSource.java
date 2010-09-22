package mobi.eyeline.informer.web.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Сторадж для журнала
 * @author Aleksandr Khalitov
 */
public interface JournalDataSource {

  /**
   * Добавление записей в журнал
   * @param records записи
   * @throws AdminException ошибка при сохранении записей
   */
  public void addRecords(JournalRecord ... records) throws AdminException;

  /**
   * Просмотр записей журнала, удовлетворяющих фильтру
   * @param filter фльтр
   * @param v посетитель
   * @throws AdminException ошибка при просмторе журнала
   */
  public void visit(JournalFilter filter, Visitor v) throws AdminException;


  /**
   * Интерфейс для просмотра записей журнала
   * @author Aleksandr Khalitov
  */
  public static interface Visitor {

    /**
     * Метод вызывается при просмотре журнала для каждой записи
     * @param r запись журнала
     * @return true - продолжать простом дальше, false - завершить просмотр
     */
    public boolean visit(JournalRecord r);

  }

}
