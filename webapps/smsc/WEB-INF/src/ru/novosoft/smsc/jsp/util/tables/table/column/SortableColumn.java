package ru.novosoft.smsc.jsp.util.tables.table.column;

import ru.novosoft.smsc.jsp.util.tables.table.Column;
import ru.novosoft.smsc.jsp.util.tables.table.SimpleTableBean;
import ru.novosoft.smsc.jsp.util.tables.table.OrderType;

/**
 * User: artem
 * Date: 21.12.2006
 */
public class SortableColumn extends Column {

  private boolean sortable;
  private OrderType orderType = null;

  public SortableColumn(SimpleTableBean simpleTableBean) {
    super(simpleTableBean);
    this.sortable = false;
  }

  public SortableColumn(SimpleTableBean simpleTableBean, boolean sortable) {
    super(simpleTableBean, sortable);
    this.sortable = sortable;
  }

  public SortableColumn(SimpleTableBean SimpleTableBean, boolean sortable, int width) {
    super(SimpleTableBean, sortable, width);
    this.sortable = sortable;
  }

  public boolean isSortable() {
    return sortable;
  }

  public void setSortable(boolean sortable) {
    this.sortable = sortable;
  }

  public OrderType getOrderType() {
    return orderType;
  }

  public void setOrderType(OrderType orderType) {
    this.orderType = orderType;
  }
}
