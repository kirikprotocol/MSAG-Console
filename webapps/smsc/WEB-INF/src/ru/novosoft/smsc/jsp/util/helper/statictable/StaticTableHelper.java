package ru.novosoft.smsc.jsp.util.helper.statictable;

import org.apache.log4j.Category;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * User: artem
 * Date: 15.01.2007
 */
public abstract class StaticTableHelper {

  private static final String SORT_ORDER_PREFIX         = "sortOrderElement";
  private static final String SELECTED_CELL_ID_PREFIX   = "selectedCellId";
  private static final String SELECTED_COLUMN_ID_PREFIX = "selectedColumnId";
  private static final String SORTED_COLUMN_ID_PREFIX   = "sortedColumnId";

  private final String uid;
  protected final Category logger;

  private final ArrayList columns = new ArrayList();
  private final ArrayList rows = new ArrayList();

  private String selectedCellId = null;
  private String selectedColumnId = null;
  private String sortedColumnId = null;

  private SortOrderElement[] sortOrder = null;

  public StaticTableHelper(final String uid, final Category logger) {
    this.uid = uid;
    this.logger = logger;
  }


//  Request processing =================================================================================================


  public void processRequest(HttpServletRequest request) throws TableHelperException {
    fillParameters(request);

    if (eventColumnSelected() != null)
      setSort(eventColumnSelected().getColumnId(), null);
  }


  protected void fillParameters(HttpServletRequest request) {
    this.selectedCellId = request.getParameter(getSelectedCellIdPrefix());
    this.selectedColumnId = request.getParameter(getSelectedColumnIdPrefix());
    this.sortedColumnId = request.getParameter(getSortedColumnIdPrefix());

    if (selectedCellId != null && selectedCellId.equals(""))
      selectedCellId = null;

    if (selectedColumnId != null && selectedColumnId.equals(""))
      selectedColumnId = null;

    if (sortedColumnId != null && sortedColumnId.equals(""))
      sortedColumnId = null;

    fillSortOrder(request);
  }

  private void fillSortOrder(HttpServletRequest request) {
    sortOrder = new SortOrderElement[columns.size()];

    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext();) {
      final String key = (String)iter.next();
      if (key.startsWith(getSortOrderPrefix())) {

        final int num = Integer.parseInt(key.substring(getSortOrderPrefix().length()));
        final String value = request.getParameter(key);
        final int start = (value.startsWith("-")) ? 1 : 0;
        final OrderType orderType = (start == 0) ? OrderType.ASC : OrderType.DESC;
        final SortOrderElement element = new SortOrderElement(value.substring(start), orderType);
        sortOrder[num] = element;
      }
    }
  }


//  Events =============================================================================================================


  public EventDataCellSelected eventDataCellSelected() {
    return (selectedColumnId != null) ? new EventDataCellSelected(selectedColumnId, selectedCellId) : null;
  }

  public EventColumnSelected eventColumnSelected() {
    return (sortedColumnId != null) ? new EventColumnSelected(sortedColumnId) : null;
  }


//  Methods ============================================================================================================


  protected void setSort(String columnId, OrderType orderType) {
    if (columnId == null)
      return;

    final SortOrderElement[] newSortOrder = new SortOrderElement[columns.size()];
    newSortOrder[0] = new SortOrderElement(columnId, (orderType == null) ? OrderType.ASC : orderType);

    if (sortOrder.length > 0 && sortOrder[0] != null && sortOrder[0].getColumnId().equals(columnId))
      newSortOrder[0].setOrderType((orderType == null) ? getOppositeOrderType(sortOrder[0].getOrderType()) : orderType);

    int j = 1;
    for (int i=0; i < sortOrder.length; i++) {
      if (sortOrder[i] != null && !sortOrder[i].getColumnId().equals(columnId)) {
        newSortOrder[j] = sortOrder[i];
        j++;
      }
    }

    sortOrder = newSortOrder;
  }

  private OrderType getOppositeOrderType(OrderType orderType) {
    return (orderType == OrderType.ASC) ? OrderType.DESC : OrderType.ASC;
  }

  protected void clear() {
    rows.clear();
  }


//  Table creation =====================================================================================================


  protected final Row createNewRow() {
    final Row row = new Row();
    rows.add(row);
    return row;
  }

  protected void addColumn(final Column column) {
    if (column != null)
      columns.add(column);
  }


//  Abstract ===========================================================================================================


  public abstract void fillTable(HttpServletRequest request) throws TableHelperException;


//  Prefixes ===========================================================================================================


  public String getSortOrderPrefix() {
    return uid + SORT_ORDER_PREFIX;
  }

  public String getSelectedCellIdPrefix() {
    return uid + SELECTED_CELL_ID_PREFIX;
  }

  public String getSelectedColumnIdPrefix() {
    return uid + SELECTED_COLUMN_ID_PREFIX;
  }

  public String getSortedColumnIdPrefix() {
    return uid + SORTED_COLUMN_ID_PREFIX;
  }


//  Properties =========================================================================================================



  public Iterator getColumns() {
    return columns.iterator();
  }

  public Iterator getRows() {
    return rows.iterator();
  }

  public int getSize() {
    return rows.size();
  }

  protected String getUId() {
    return uid;
  }

  public SortOrderElement[] getSortOrder() {
    return sortOrder;
  }

  public OrderType getOrderType(String columnId) {
    if (sortOrder.length == 0 || columnId == null)
      return null;
    final SortOrderElement element = (SortOrderElement)sortOrder[0];
    return (element != null && element.getColumnId().equals(columnId)) ? element.getOrderType() : null;
  }


//  Sort order element =================================================================================================


  public class SortOrderElement {
    private final String columnId;
    private OrderType orderType;

    public SortOrderElement(String columnId, OrderType orderType) {
      this.columnId = columnId;
      this.orderType = orderType;
    }

    public String getColumnId() {
      return columnId;
    }

    public OrderType getOrderType() {
      return orderType;
    }

    public void setOrderType(OrderType orderType) {
      this.orderType = orderType;
    }
  }


//  Event data cell selected ===========================================================================================


  public class EventDataCellSelected {
    private final String columnId;
    private final String cellId;

    private EventDataCellSelected(String columnId, String cellId) {
      this.columnId = columnId;
      this.cellId = cellId;
    }

    public String getColumnId() {
      return columnId;
    }

    public String getCellId() {
      return cellId;
    }
  }


//  Event column selected ==============================================================================================


  public class EventColumnSelected {
    private final String columnId;

    private EventColumnSelected(String columnId) {
      this.columnId = columnId;
    }

    public String getColumnId() {
      return columnId;
    }
  }


}
