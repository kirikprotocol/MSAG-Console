package ru.novosoft.smsc.jsp.util.tables.impl.provider;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

public class ProviderQuery extends AbstractQueryImpl
{
  public ProviderQuery(int expectedResultsQuantity, ProviderFilter filter, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
