package ru.sibinco.smppgw.backend.protocol.tables.impl;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:33:24
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.smppgw.backend.protocol.tables.Query;
import ru.sibinco.smppgw.backend.protocol.tables.Filter;

import java.util.Vector;


public abstract class AbstractQueryImpl implements Query
{
  protected int expectedResultsQuantity = 10;
  protected Filter filter = null;
  protected Vector sortOrder = null;
  protected int startPosition = 0;

  public AbstractQueryImpl(int expectedResultsQuantity, Filter filter, Vector sortOrder, int startPosition)
  {
    this.expectedResultsQuantity = expectedResultsQuantity;
    this.filter = filter;
    this.sortOrder = sortOrder;
    this.startPosition = startPosition;
  }

  public AbstractQueryImpl(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition)
  {
    this.expectedResultsQuantity = expectedResultsQuantity;
    this.filter = filter;
    this.sortOrder = new Vector(1);
    this.sortOrder.add(sortOrder);
    this.startPosition = startPosition;
  }

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
