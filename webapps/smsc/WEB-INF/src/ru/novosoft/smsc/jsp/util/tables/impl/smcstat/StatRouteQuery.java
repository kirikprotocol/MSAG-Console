package ru.novosoft.smsc.jsp.util.tables.impl.smcstat;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

public class StatRouteQuery extends AbstractQueryImpl
{
  public StatRouteQuery(int expectedResultsQuantity, StatRouteFilter filter, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
