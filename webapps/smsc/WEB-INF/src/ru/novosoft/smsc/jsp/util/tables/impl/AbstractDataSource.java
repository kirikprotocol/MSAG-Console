package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import java.util.Vector;

/**
 * User: artem
 * Date: 26.11.2008
 */
public abstract class AbstractDataSource implements DataSource {

  protected final String[] columnNames;

  private DataItemList data;
  private boolean isNegativeSort = false;
  private Query query;
  private int totalSize;


  protected AbstractDataSource(String[] columnNames) {
    this.columnNames = columnNames;
  }

  public String[] getColumnNames() {
    return columnNames;
  }

  protected void init(Query query) {
    this.query = query;
    String  sort = (String) ((query.getSortOrder() == null || query.getSortOrder().isEmpty()) ? null : query.getSortOrder().get(0));
    if (sort != null && sort.length() > 0 && sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }
    data = new DataItemList(sort, isNegativeSort, query.getStartPosition() + query.getExpectedResultsQuantity());
  }

  protected void add(DataItem item) throws AdminException {
    if (data == null)
      throw new AdminException("Data source is not initialized.");

    if ((query.getFilter() == null || query.getFilter().isItemAllowed(item))) {
      totalSize++;
      data.add(item);
    }
  }

  protected QueryResultSet getResults() {
    QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, query.getSortOrder());
    rs.setResults(data.results(query.getStartPosition(), query.getExpectedResultsQuantity()));
    rs.setTotalSize(totalSize);

    data = null;
    query = null;
    totalSize = 0;

    return rs;
  }



  private static class DataItemList {

    private final Element head;
    private final int maxSize;
    private final DataItemComparator comparator;
    private int size;

    private DataItemList(String sort, boolean reverse, int maxSize) {
      this.comparator = sort == null ? null : new DataItemComparator(sort, reverse);
      this.size = 0;
      this.maxSize = maxSize;
      this.head = new Element();
      head.next = head;
      head.prev = head;
    }

    public void add(DataItem item) {

      if (comparator != null) {

        Element backward = head.prev;
        Element forward = head.next;

        while(true) {

          if (backward == head || comparator.compare(backward.value, item) < 0) {
            if (size < maxSize || backward.next != head)
              _insert(item, backward);
            break;

          } else if (forward == head || comparator.compare(forward.value, item) >= 0) {
            if (size < maxSize || forward.prev.next != head)
              _insert(item, forward.prev);                          
            break;
          }
          backward = backward.prev;
          forward = forward.next;
        }

        if (size > maxSize) { // Remove last element in list
          head.prev.prev.next = head;
          head.prev = head.prev.prev;
          size--;
        }

      } else {
        if (size < maxSize)
          _insert(item, head.prev);
      }
    }

    private void _insert(DataItem item, Element cur) {
      Element el = new Element();
      el.value = item;
      
      el.prev = cur;
      el.next = cur.next;

      cur.next.prev = el;
      cur.next = el;

      size++;
    }

    public Vector results(int startPos, int size) {
      final Vector l = new Vector(size+1);
      Element e = head.next;
      int i=0;
      while (e != head && i < startPos + size) {
        if (i >= startPos)
          l.add(e.value);
        e = e.next;
        i++;
      }
      return l;
    }

    public Vector reversedResults(int startPos, int size) {
      final Vector l = new Vector(size+1);
      Element e = head.prev;
      int i=0;
      while (e != head && i < startPos + size) {
        if (i >= startPos)
          l.add(e.value);
        e = e.prev;
        i++;
      }
      return l;
    }

    private static class Element {
      private DataItem value;
      private Element next;
      private Element prev;
    }
  }
}
