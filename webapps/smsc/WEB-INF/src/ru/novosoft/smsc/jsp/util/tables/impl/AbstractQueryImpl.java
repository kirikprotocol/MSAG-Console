/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:12:30
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.Vector;


abstract class AbstractQueryImpl implements Query
{
  protected int expectedResultsQuantity = 10;
  protected Filter filter = null;
  protected Vector sortOrder = null;
  protected int startPosition = 0;

  public int getExpectedResultsQuantity()
  {
    return expectedResultsQuantity;
  }

  public Filter getFilter()
  {
    return filter;
  }

  public Vector getSortOrder()
  {
    return sortOrder;
  }

  public int getStartPosition()
  {
    return startPosition;
  }
}
