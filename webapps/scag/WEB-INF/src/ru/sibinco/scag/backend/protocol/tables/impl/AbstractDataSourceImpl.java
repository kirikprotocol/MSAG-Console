package ru.sibinco.scag.backend.protocol.tables.impl;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:56:36
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.protocol.tables.*;
import ru.sibinco.lib.backend.util.SortedVector;

import java.util.*;


public abstract class AbstractDataSourceImpl implements DataSource
{
  protected Vector data = new Vector();
  protected Map sorts = new HashMap();
  protected String[] columnNames = null;

  protected AbstractDataSourceImpl(String[] columnNames)
  {
    this.columnNames = columnNames;
    for (int i = 0; i < columnNames.length; i++) {
      sorts.put(columnNames[i], new SortedVector(new DataItemComparator(columnNames[i])));
    }
  }

  public String[] getColumnNames()
  {
    return columnNames;
  }

  protected void add(DataItem item)
  {
    data.add(item);
    for (Iterator i = sorts.values().iterator(); i.hasNext();) {
      ((SortedVector) i.next()).add(item);
    }
  }

  protected void remove(DataItem item)
  {
    data.remove(item);
    for (Iterator i = sorts.values().iterator(); i.hasNext();) {
      ((SortedVector) i.next()).remove(item);
    }
  }

  public void clear()
  {
    data.clear();
    for (Iterator i = sorts.values().iterator(); i.hasNext();) {
      ((SortedVector) i.next()).clear();
    }
  }

  public QueryResultSet query(Query query_to_run)
  {
    String sort = (String) query_to_run.getSortOrder().get(0);
    boolean isNegativeSort = false;
    if (sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }

    Vector srcVector = data;
    if (query_to_run.getSortOrder().size() > 0)
      srcVector = (SortedVector) sorts.get(sort);
    if (query_to_run.getSortOrder().size() > 1)
      throw new UnsupportedOperationException("Hierarchical sorts not implemented");

    QueryResultSetImpl results = new QueryResultSetImpl(columnNames, query_to_run.getSortOrder());

    Filter filter = query_to_run.getFilter();

    if (isNegativeSort) {
      Vector reversedSrcVector = new Vector(srcVector);
      Collections.reverse(reversedSrcVector);
      srcVector = reversedSrcVector;
    }

    Iterator i = srcVector.iterator();
    final int start_position = query_to_run.getStartPosition();
    int totalCount = 0;
    for (int count_to_skip = 0; i.hasNext() && count_to_skip < start_position;) {
      DataItem item = (DataItem) i.next();
      if (filter.isItemAllowed(item)) {
        count_to_skip++;
        totalCount++;
      }

    }

    final int quantity = query_to_run.getExpectedResultsQuantity();
    for (int count = 0; i.hasNext() && count < quantity;) {
      DataItem item = (DataItem) i.next();
      if (filter.isItemAllowed(item)) {
        count++;
        totalCount++;
        results.add(item);
      }
    }

    results.setLast(true);
    for (; i.hasNext();) {
      if (filter.isItemAllowed((DataItem) i.next())) {
        results.setLast(false);
        totalCount++;
      }
    }
    results.setTotalSize(totalCount);

    return results;
  }
}