package ru.novosoft.smsc.jsp.util.tables.table;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCJspException;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * User: artem
 * Date: 18.12.2006
 */

public abstract class SimpleTableBean extends PageBean {

  public static final String SORT_ORDER_PREFIX = "sortOrderElement";

  private final ArrayList columns = new ArrayList();
  private final ArrayList rows = new ArrayList();

  private String selectedCellId = null;
  private String selectedColumnId = null;

  private SortOrderElement[] sortOrder = null;

  public final int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    sortOrder = new SortOrderElement[columns.size()];
    fillSortOrder(request);

    result = checkDataCellSelected(request);

    if (result != RESULT_OK && result != RESULT_ERROR)
      return result;

    if (result != RESULT_ERROR)
      result = checkColumnSelected(request);

    if (result != RESULT_OK && result != RESULT_ERROR)
      return result;

    if (result != RESULT_ERROR)
      result = processRequest(request);

    selectedCellId = null;
    selectedColumnId = null;

    try {
      fillTable(request);
    } catch (AdminException e) {
      logger.error("Can't fill table", e);
      return _error(new SMSCJspException("Can't fill table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return result;
  }

  protected void clear() {
    rows.clear();
  }

  protected int checkDataCellSelected(HttpServletRequest request) {
    return (selectedCellId != null && selectedColumnId != null) ? onCellSelected(request, selectedColumnId, selectedCellId) : RESULT_OK;
  }

  protected int checkColumnSelected(HttpServletRequest request) {
    return (selectedColumnId != null && selectedCellId == null) ? onColumnSelected(request, selectedColumnId) : RESULT_OK;
  }

  protected abstract int processRequest(HttpServletRequest request);

  protected abstract void fillTable(HttpServletRequest request) throws AdminException;

  protected abstract int onCellSelected(final HttpServletRequest request, final String columnId, final String cellId);

  protected int onColumnSelected(final HttpServletRequest request, final String columnId) {
    setSort(columnId, null);
    return RESULT_OK;
  }

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

  private void fillSortOrder(HttpServletRequest request) {
    for (Iterator iter = request.getParameterMap().keySet().iterator(); iter.hasNext();) {
      final String key = (String)iter.next();
      if (key.startsWith(SORT_ORDER_PREFIX)) {

        final int num = Integer.parseInt(key.substring(SORT_ORDER_PREFIX.length()));
        final String value = request.getParameter(key);
        final int start = (value.startsWith("-")) ? 1 : 0;
        final OrderType orderType = (start == 0) ? OrderType.ASC : OrderType.DESC;
        final SortOrderElement element = new SortOrderElement(value.substring(start), orderType);
        sortOrder[num] = element;
      }
    }
  }

  public OrderType getOrderType(String columnId) {
    if (sortOrder.length == 0 || columnId == null)
      return null;
    final SortOrderElement element = (SortOrderElement)sortOrder[0];
    return (element != null && element.getColumnId().equals(columnId)) ? element.getOrderType() : null;
  }



  void addColumn(final Column column) {
    if (column != null)
      columns.add(column);
  }

  public Iterator getColumns() {
    return columns.iterator();
  }

  protected final Row createNewRow() {
    final Row row = new Row();
    rows.add(row);
    return row;
  }

  public Iterator getRows() {
    return rows.iterator();
  }


  public int getSize() {
    return rows.size();
  }


  public void setSelectedCellId(String selectedCellId) {
    this.selectedCellId = selectedCellId;
  }

  public void setSelectedColumnId(String selectedColumnId) {
    this.selectedColumnId = selectedColumnId;
  }

  public SortOrderElement[] getSortOrder() {
    return sortOrder;
  }

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

}
