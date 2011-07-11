package mobi.eyeline.util.jsf.components.data_table.model;

/**
 * author: Aleksandr Khalitov
 * Позволяет по объекту получить его идентификатор
 * Данный интерфейс используется в DataTable для выделения строк
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
