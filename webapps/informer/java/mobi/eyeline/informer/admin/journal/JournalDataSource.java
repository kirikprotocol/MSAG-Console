package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Сторадж для журнала
 *
 * @author Aleksandr Khalitov
 */
interface JournalDataSource {

  /**
   * Добавление записей в журнал
   *
   * @param records записи
   * @throws AdminException ошибка при сохранении записей
   */
  void addRecords(JournalRecord... records) throws AdminException;

  /**
   * Просмотр записей журнала, удовлетворяющих фильтру
   *
   * @param filter фльтр
   * @param v      посетитель
   * @throws AdminException ошибка при просмторе журнала
   */
  void visit(JournalFilter filter, JournalVisitor v) throws AdminException;


}
