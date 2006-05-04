package ru.novosoft.smsc.jsp.util.tables.impl;

/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 17:15:42
 */

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import java.util.Collections;
import java.util.Iterator;
import java.util.Vector;


public class QueryResultSetImpl implements QueryResultSet
{
  private Vector results = new Vector();
  private String[] columns = null;
  private Vector sortOrder = null;
  private boolean last = false;
  private int totalSize = 0;

  public QueryResultSetImpl()
  {
    this.columns = new String[0];
  }

  public QueryResultSetImpl(String[] columns, Vector sortOrder)
  {
    this.columns = columns;
    this.sortOrder = sortOrder;
  }

  public QueryResultSetImpl(String[] columns, String sortOrder)
  {
    this.columns = columns;
    this.sortOrder = new Vector();
    this.sortOrder.add(sortOrder);
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

  public void setSortOrder(Vector sortOrder){
    this.sortOrder = sortOrder;
  }

  public boolean isLast()
  {
    return last;
  }

  public void additionalSort(String columnName)
  {
    String sortColumn = (String) sortOrder.get(0);
    if (sortColumn.startsWith("-"))
      sortColumn = sortColumn.substring(1);

    Vector newResults = new Vector(results.size());
    int blockEnd = 0;
    for (int blockStart = 0; blockStart < results.size(); blockStart = blockEnd) {
      blockEnd = getBlockEnd(blockStart, sortColumn);
      Vector block = new Vector(blockEnd - blockStart);
      for (int i = blockStart; i < blockEnd; i++)
        block.add(results.get(i));
      Collections.sort(block, new DataItemComparator(columnName));
      newResults.addAll(block);
    }
    results = newResults;
  }

  public void sortByColumnName(String columnName) {
      Collections.sort(results, new DataItemComparator(columnName));
  }

  private int getBlockEnd(int blockStart, String columnName)
  {
    System.out.println("QueryResultSetImpl.getBlockEnd(\"" + columnName + "\")");
    DataItem startItem = (DataItem) results.get(blockStart);
    Object startValue = startItem.getValue(columnName);
    for (int i = blockStart + 1; i < results.size(); i++) {
      System.out.println("i = " + i);
      DataItem dataItem = (DataItem) results.get(i);
      System.out.println("dataItem = " + dataItem);
      if (dataItem != null)
        //System.out.println("dataItem.getValue(columnName) = " + dataItem.getValue(columnName));
      if (!dataItem.getValue(columnName).equals(startValue))
        return i;
    }
    return results.size();
  }

  public void setLast(boolean last)
  {
    this.last = last;
  }

  public int getTotalSize()
  {
    return totalSize;
  }

  public void setTotalSize(int totalSize)
  {
    this.totalSize = totalSize;
  }

}
