/*
 * Author: igork
 * Date: 07.06.2002
 * Time: 17:41:06
 */
package ru.novosoft.smsc.jsp.util.tables;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;


public interface QueryResultSet
{
  int size();
  DataItem get(int index);
  Iterator iterator();
  String[] getColumns();
  Vector getSortOrder();
}
