package ru.novosoft.smsc.web.components.paged_table;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public interface PagedTableModel {

  List<PagedTableRow> getRows(int startPos, int count, PagedTableSortOrder sortOrder);

  void setSelectedRows(int[] rows);

  int getRowsCount() ;
}