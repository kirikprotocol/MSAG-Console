package mobi.eyeline.util.jsf.components.data_table.model;

import java.util.List;

/**
 * Интерфейс, предоставляющий доступ к данным одной строки Data Table
 *
 * @author Artem Snopkov
 */
public interface DataTableRow {

  /**
   * Возвращает уникальный идентификатор строки.
   *
   * @return уникальный идентификатор строки.
   */
  public String getId();

  /**
   * Возвращает данные, которые надо отобразить в основной таблице
   *
   * @return данные, которые надо отобразить в основной таблице
   */
  public Object getData();

  /**
   * Возвращает объект, вложенный в данную строку или null, если такого нет
   *
   * @return объект, вложенный в данную строку или null, если такого нет
   */
  public Object getInnerData();

  /**
   * Возвращает список строк таблицы, вложенных в данную строку или null, если таких нет
   *
   * @return список строк таблицы, вложенных в данную строку или null, если таких нет
   */
  public List<Object> getInnerRows();

}
