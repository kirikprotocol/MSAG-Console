package ru.novosoft.smsc.jsp.util.tables;

/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 16:57:10
 */

import java.util.Vector;


public interface Query
{
  Filter getFilter();

  int getStartPosition();

  int getExpectedResultsQuantity();

  Vector getSortOrder();
}
