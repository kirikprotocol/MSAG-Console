package ru.novosoft.smsc.jsp.util.tables;

/*
 * Author: igork
 * Date: 07.06.2002
 * Time: 17:41:06
 */

import java.util.Iterator;
import java.util.Vector;


public interface QueryResultSet
{
  int size();

  int getTotalSize();

  DataItem get(int index);

  Iterator iterator();

  String[] getColumns();

  Vector getSortOrder();

  void setSortOrder(Vector sortOrder);

  public void sortBycolumnName(String columnName);

  boolean isLast();

  void additionalSort(String columnName);
}
