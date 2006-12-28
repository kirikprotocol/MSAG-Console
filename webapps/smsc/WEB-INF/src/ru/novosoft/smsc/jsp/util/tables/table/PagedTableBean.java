package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.tables.table.column.SortableColumn;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 20.12.2006
 */
public abstract class PagedTableBean extends SimpleTableBean {

  private int startPosition = 0;
  private int totalSize = 0;
  private int pageSize = 1;

  protected String mbFilter = null;

  private boolean showFilter = false;

  private SortableColumn sortedColumn = null;

  protected final int processRequestInternal(HttpServletRequest request) {
    if (showFilter && mbFilter != null) {
      mbFilter = null;
      return showFilter(request);
    }
    return doProcess(request);
  }

  protected abstract int doProcess(HttpServletRequest request);

  protected final void fillTable() throws AdminException {
    clear();
    fillTable(startPosition, pageSize);
    totalSize = calculateTotalSize();
  }

  protected final int onColumnSelected(final Column column) {
    if (!(column instanceof SortableColumn))
      return onColumnSelect(column);

    if (sortedColumn == column)
      sortedColumn.setOrderType(getOppositeOrderType(sortedColumn.getOrderType()));
    else {
      if (sortedColumn != null)
        sortedColumn.setOrderType(null);
      sortedColumn = (SortableColumn)column;
      sortedColumn.setOrderType(OrderType.ASC);
    }

    try {
      fillTable();
    } catch (AdminException e) {
      logger.error("Can't fill table", e);
      return _error(new SMSCJspException("Can't fill table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return RESULT_DONE;
  }

  private OrderType getOppositeOrderType(OrderType orderType) {
    return (orderType == OrderType.ASC) ? OrderType.DESC : OrderType.ASC;
  }

  public SortableColumn getSortedColumn() {
    return sortedColumn;
  }

  protected void setSort(SortableColumn column) {
    this.sortedColumn = column;
    sortedColumn.setOrderType(getOppositeOrderType(sortedColumn.getOrderType()));
  }

  protected int onColumnSelect(Column column) {
    return RESULT_OK;
  }


  protected abstract void fillTable(int start, int size) throws AdminException;

  protected abstract int calculateTotalSize();

  protected int showFilter(HttpServletRequest request) {
    return PRIVATE_RESULT;
  }

  protected final void enableFilter() {
    showFilter = true;
  }

  protected final void disableFilter() {
    showFilter = false;
  }

  public boolean isShowFilter() {
    return showFilter;
  }




  public String getStartPosition() {
    return String.valueOf(startPosition);
  }

  public int getStartPositionInt() {
    return startPosition;
  }

  public void setStartPosition(String startPosition) {
    try {
      this.startPosition = Integer.decode(startPosition).intValue();
    } catch (NumberFormatException e) {
      this.startPosition = 0;
    }
  }


  public String getTotalSize() {
    return String.valueOf(totalSize);
  }

  public int getTotalSizeInt() {
    return totalSize;
  }

  public void setTotalSize(int totalSize) {
    this.totalSize = totalSize;
  }


  public String getPageSize() {
    return String.valueOf(pageSize);
  }

  public int getPageSizeInt() {
    return pageSize;
  }

  public void setPageSize(int pageSize) {
    this.pageSize = pageSize;
  }

  public void setMbFilter(String mbFilter) {
    this.mbFilter = mbFilter;
  }
}
