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

  public PagedStaticTableHelper(String uid, boolean filterEnabled) {
    super(uid);
    this.filterEnabled = filterEnabled;
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


//  Events =============================================================================================================


  public EventShowFilter eventShowFilter() {
    return (mbFilter != null) ? new EventShowFilter() : null;
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


//  Event show filter ==================================================================================================


  public class EventShowFilter {
    private EventShowFilter() {
    }
  }


}
