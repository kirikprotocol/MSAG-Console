package ru.novosoft.smsc.jsp.util.tables.impl.mscman;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * Created by igork
 * Date: Aug 18, 2003
 * Time: 3:07:06 PM
 */
public class MscQuery extends AbstractQueryImpl
{
  public MscQuery(int expectedResultsQuantity, MscFilter filter, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
