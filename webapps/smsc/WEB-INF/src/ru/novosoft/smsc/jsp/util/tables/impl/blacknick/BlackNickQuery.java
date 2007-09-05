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

public class BlackNickQuery implements Query {

  public static final byte MATCH_TYPE_EXACTLY = 0;
  public static final byte MATCH_TYPE_PREFIX = 1;

  private int expectedResultsQuantity;
  private Filter filter;
  private Vector sortOrder;

  public BlackNickQuery(int expectedResultsQuantity, final String nickFilter, byte matchType) {
    this.expectedResultsQuantity = expectedResultsQuantity;
    this.filter = new BlackNickFilter(nickFilter, matchType);
    this.sortOrder = new Vector();
    this.sortOrder.add("nick");
  }

  public BlackNickQuery(int expectedResultsQuantity, final String nickFilter, String sortOrder) {
    this.expectedResultsQuantity = expectedResultsQuantity;
    this.filter = new BlackNickFilter(nickFilter, MATCH_TYPE_PREFIX);
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

  private static class BlackNickFilter implements Filter {
    private String nickFilter;
    private byte matchType;

    public BlackNickFilter(String nickFilter, byte matchType) {
      this.nickFilter = nickFilter;
      this.matchType = matchType;
    }

    public boolean isEmpty() {
        return nickFilter == null;
      }
      public boolean isItemAllowed(DataItem item) {
        if (nickFilter == null)
          return true;

        if (item.getValue("nick") == null)
          return false;

        if (matchType == MATCH_TYPE_PREFIX)
          return item.getValue("nick").toString().startsWith(nickFilter);
        else if (matchType == MATCH_TYPE_EXACTLY)
          return item.getValue("nick").toString().equalsIgnoreCase(nickFilter);

        return false;
      }
  }

}
