package ru.novosoft.smsc.emailsme.backend;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 17:02:42
 */
public class ProfilesQuery extends AbstractQueryImpl
{
  public ProfilesQuery(int expectedResultsQuantity, ProfilesFilter filter, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
