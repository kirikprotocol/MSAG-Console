package ru.novosoft.smsc.web.components.paged_table;

/**
 * @author Artem Snopkov
 */
public class PagedTableSortOrder {

  private final String columnId;
  private final boolean asc;

  public PagedTableSortOrder(String columnId, boolean asc) {
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
