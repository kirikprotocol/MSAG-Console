package mobi.eyeline.util.jsf.components.data_table.model;

/**
 * author: Aleksandr Khalitov
 * Модель, позволяющая определить идентификаторы объектов, возвращаемых методом getRows().
 * Если в таблице требуется выделение строк, то её модель должна имплементировать данный интерфейс.
 */
public interface ModelWithObjectIds extends DataTableModel {

  /**
   * Позволяет по объекту получить его идентификатор
   * @param o объект
   * @return идентификатор
   * @throws ModelException ошибка при получении идентификатора
   */
  public String getId(Object o) throws ModelException;
}
