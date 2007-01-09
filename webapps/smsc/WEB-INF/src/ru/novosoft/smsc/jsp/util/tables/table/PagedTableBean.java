package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 20.12.2006
 */
public abstract class PagedTableBean extends SimpleTableBean {

  private int startPosition = 0;
  private int totalSize = 0;
  private int pageSize = 1;

  private String mbFilter = null;
  private boolean filterEnabled = false;


  protected final int processRequest(HttpServletRequest request) {
    if (filterEnabled && mbFilter != null) {
      mbFilter = null;
      return showFilter(request);
    }
    return doProcess(request);
  }

  protected abstract int doProcess(HttpServletRequest request);

  protected final void fillTable(HttpServletRequest request) throws AdminException {
    fillTable(request, startPosition, pageSize);
    totalSize = calculateTotalSize();
  }

  protected abstract void fillTable(HttpServletRequest request, int start, int size) throws AdminException;

  protected abstract int calculateTotalSize();

  protected int showFilter(HttpServletRequest request) {
    return PRIVATE_RESULT;
  }

  public boolean isFilterEnabled() {
    return filterEnabled;
  }

  public void setFilterEnabled(boolean filterEnabled) {
    this.filterEnabled = filterEnabled;
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

  protected void setStartPosition(int startPosition) {
    this.startPosition = startPosition;
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
