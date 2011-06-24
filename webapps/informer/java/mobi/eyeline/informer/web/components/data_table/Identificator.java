package mobi.eyeline.informer.web.components.data_table;

/**
 * author: Aleksandr Khalitov
 * Позволяет по объекту получить его идентификатор
 * Данный интерфейс используется в DataTable для выделения строк
 */
public interface Identificator {

  /**
   * Позволяет по объекту получить его идентификатор
   * @param o объект
   * @return идентификатор
   */
  public String getId(Object o);
}
