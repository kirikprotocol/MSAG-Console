package ru.novosoft.smsc.jsp.util.tables.impl.dl;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 22.09.2003
 * Time: 16:22:29
 * To change this template use Options | File Templates.
 */
public class PrincipalsQuery extends AbstractQueryImpl
{
  public PrincipalsQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
