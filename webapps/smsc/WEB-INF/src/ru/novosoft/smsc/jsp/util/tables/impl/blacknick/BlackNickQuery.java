package ru.novosoft.smsc.jsp.util.tables.impl.blacknick;

import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.Aggregator;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Vector;

/**
 * User: artem
 * Date: 18.07.2007
 */

public class BlackNickQuery implements Query{

  private int expectedResultsQuantity;
  private Filter filter;
  private Vector sortOrder;

  public BlackNickQuery(int expectedResultsQuantity, final String nickFilter, String sortOrder) {
    this.expectedResultsQuantity = expectedResultsQuantity;
    this.filter = new Filter() {
      public boolean isEmpty() {
        return nickFilter == null;
      }
      public boolean isItemAllowed(DataItem item) {
        if (nickFilter == null)
          return true;
        
        return item.getValue("nick") != null && item.getValue("nick").toString().startsWith(nickFilter);
      }
    };
    this.sortOrder = new Vector();
    if (sortOrder != null)
      this.sortOrder.add(sortOrder);
  }

  public int getExpectedResultsQuantity() {
    return expectedResultsQuantity;
  }

  public void setExpectedResultsQuantity(int expectedResultsQuantity) {
    this.expectedResultsQuantity = expectedResultsQuantity;
  }

  public Aggregator getAggregator() {
    return null;
  }

  public Filter getFilter() {
    return filter;
  }

  public int getStartPosition() {
    return 0;
  }

  public Vector getSortOrder() {
    return sortOrder;
  }

}
