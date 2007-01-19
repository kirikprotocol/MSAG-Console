package ru.novosoft.smsc.jsp.util.helper.statictable;

import org.apache.log4j.Category;

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

  public PagedStaticTableHelper(String uid, boolean filterEnabled, Category logger) {
    super(uid, logger);
    this.filterEnabled = filterEnabled;
  }


//  Request processing =================================================================================================


  protected void fillParameters(HttpServletRequest request) {
    super.fillParameters(request);

    try {
      if (request.getParameter(getStartPositionPrefix()) != null)
        this.startPosition = Integer.parseInt(request.getParameter(getStartPositionPrefix()));
    } catch (NumberFormatException e) {
      logger.error("Can't set start position", e);
    }

    this.mbFilter = request.getParameter(getMbFilterPrefix());
  }

  public final void fillTable(HttpServletRequest request) throws TableHelperException {
    fillTable(request, startPosition, pageSize);
    totalSize = calculateTotalSize();
  }


//  Events =============================================================================================================


  public EventShowFilter eventShowFilter() {
    return (mbFilter != null) ? new EventShowFilter() : null;
  }

//  Abstract ===========================================================================================================


  protected abstract void fillTable(HttpServletRequest request, int start, int size) throws TableHelperException;

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
