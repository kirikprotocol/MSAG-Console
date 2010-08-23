package ru.novosoft.smsc.web.components.data_table.model;

/**
 * Класс, описывающий порядок сортировки в DataTable
 * @author Artem Snopkov
 */
public class DataTableSortOrder {

  private final String columnId;
  private final boolean asc;

  public DataTableSortOrder(String columnId, boolean asc) {
    this.columnId = columnId;
    this.asc = asc;
  }

  /**
   * Возвращает название колонки, по которой требуется отсортировать данные
   * @return название колонки, по которой требуется отсортировать данные
   */
  public String getColumnId() {
    return columnId;
  }

  /**
   * Порядок сортировки
   * @return true, если надо отсортировать по-возрастанию, false - по-убыванию
   */
  public boolean isAsc() {
    return asc;
  }
}
