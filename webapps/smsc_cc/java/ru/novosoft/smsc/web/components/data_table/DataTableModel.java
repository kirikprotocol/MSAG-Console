package ru.novosoft.smsc.web.components.data_table;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public abstract class DataTableModel {

  private String sortOrder;

  public abstract List<DataTableRow> getRows(int startPos, int count, DataTableSortOrder sortOrder);

  public abstract void setSelectedRows(int[] rows);

  public abstract int getRowsCount() ;

  void setSortOrder(String sortOrder) {
    this.sortOrder = sortOrder;
  }

  String getSortOrder() {
    return sortOrder;
  }
}
