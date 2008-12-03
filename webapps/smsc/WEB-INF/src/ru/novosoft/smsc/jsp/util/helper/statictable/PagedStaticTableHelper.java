package ru.novosoft.smsc.jsp.util.helper.statictable;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 15.01.2007
 */

public abstract class PagedStaticTableHelper extends StaticTableHelper {

  private static final String START_POSITION_PREFIX = "startPosition";
  private static final String MB_FILTER_PREFIX = "mbFilter";

  private int startPosition = 0;
  private int totalSize = 0;
  private int pageSize = 1;

  private String mbFilter = null;
  private final boolean filterEnabled;
  private int maxRows;

  public PagedStaticTableHelper(String uid, boolean filterEnabled) {
    this(uid, filterEnabled, -1);
  }

  public PagedStaticTableHelper(String uid, boolean filterEnabled, int maxRows) {
    super(uid);
    this.filterEnabled = filterEnabled;
    this.maxRows = maxRows;
  }


//  Request processing =================================================================================================


  protected void fillParameters(HttpServletRequest request) {
    super.fillParameters(request);

    try {
      if (request.getParameter(getStartPositionPrefix()) != null)
        this.startPosition = Integer.parseInt(request.getParameter(getStartPositionPrefix()));
    } catch (NumberFormatException e) {
      this.startPosition = 0;
    }

    this.mbFilter = request.getParameter(getMbFilterPrefix());
  }

  public final void fillTable() throws TableHelperException {
    fillTable(startPosition, pageSize);
    totalSize = calculateTotalSize();
  }


//  Abstract ===========================================================================================================


  protected abstract void fillTable(int start, int size) throws TableHelperException;

  protected abstract int calculateTotalSize() throws TableHelperException;


//  Prefixes ===========================================================================================================


  public String getStartPositionPrefix() {
    return getUId() + START_POSITION_PREFIX;
  }

  public String getMbFilterPrefix() {
    return getUId() + MB_FILTER_PREFIX;
  }


//  Properties =========================================================================================================


  public boolean isFilterEnabled() {
    return filterEnabled;
  }

  public int getStartPosition() {
    return startPosition;
  }

  public void setStartPosition(int startPosition) {
    this.startPosition = startPosition;
  }

  public int getTotalSize() {
    return totalSize;
  }

  public void setTotalSize(int totalSize) {
    this.totalSize = totalSize;
  }

  public int getPageSize() {
    return pageSize;
  }

  public void setPageSize(int pageSize) {
    this.pageSize = pageSize;
  }

  public boolean isShowFilter() {
    return mbFilter != null;
  }

  public int getMaxRows() {
    return maxRows;
  }

  public void setMaxRows(int maxRows) {
    this.maxRows = maxRows;
  }
}
