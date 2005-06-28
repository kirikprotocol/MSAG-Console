package ru.sibinco.scag.backend.protocol.tables;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 20:01:28
 * To change this template use File | Settings | File Templates.
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

  boolean isLast();

  void additionalSort(String columnName);
}
