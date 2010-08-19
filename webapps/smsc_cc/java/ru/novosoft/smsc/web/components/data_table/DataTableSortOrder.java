package ru.novosoft.smsc.web.components.data_table;

/**
 * @author Artem Snopkov
 */
public class DataTableSortOrder {

  private final String columnId;
  private final boolean asc;

  public DataTableSortOrder(String columnId, boolean asc) {
    this.columnId = columnId;
    this.asc = asc;
  }

  public String getColumnId() {
    return columnId;
  }

  public boolean isAsc() {
    return asc;
  }
}
