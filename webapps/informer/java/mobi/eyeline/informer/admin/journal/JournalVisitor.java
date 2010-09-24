package mobi.eyeline.informer.admin.journal;

/**
 * Интерфейс для просмотра записей журнала
 * @author Aleksandr Khalitov
*/
public interface JournalVisitor {

  /**
   * Метод вызывается при просмотре журнала для каждой записи
   * @param r запись журнала
   * @return true - продолжать простом дальше, false - завершить просмотр
   */
  public boolean visit(JournalRecord r);

}
