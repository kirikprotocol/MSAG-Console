/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 17:15:42
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;
import java.util.Collections;


class QueryResultSetImpl implements QueryResultSet
{
  private Vector results = new Vector();
  private String[] columns = null;
  private Vector sortOrder = null;

  public QueryResultSetImpl(String[] columns, Vector sortOrder)
  {
    this.columns = columns;
    this.sortOrder = sortOrder;
  }

  public Iterator iterator()
  {
    return results.iterator();
  }

  public void add(DataItem item)
  {
    results.add(item);
  }

  public int size()
  {
    return results.size();
  }

  public DataItem get(int index)
  {
    return (DataItem) results.get(index);
  }

  public String[] getColumns()
  {
    return columns;
  }

  public Vector getSortOrder()
  {
    return sortOrder;
  }

  /*public void reverseOrder()
  {
    Collections.reverse(results);
  }*/
}
