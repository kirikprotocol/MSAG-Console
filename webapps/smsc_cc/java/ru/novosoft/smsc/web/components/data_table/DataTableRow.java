package ru.novosoft.smsc.web.components.data_table;

/**
 * Интерфейс, предоставляющий доступ к данным одной строки Data Table
 * @author Artem Snopkov
 */
public interface DataTableRow {

  /**
   * Возвращает уникальный идентификатор строки.
   * @return уникальный идентификатор строки.
   */
  public String getId();

  /**
   * Возвращает данные, которые надо передать в указанную колонку
   * @param columnName название колонки
   * @return
   */
  public Object getData(String columnName);

  /**
   * Возвращает текст, который надо дополнительно отобразить в строке
   * @return текст, который надо дополнительно отобразить в строке
   */
  public Object getInnerText();

}
