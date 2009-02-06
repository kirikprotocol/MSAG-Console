package ru.novosoft.smsc.infosme.backend.tables.tasks;

import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 1:19:58 PM
 */
public class TaskQuery extends AbstractQueryImpl
{
  public TaskQuery(int expectedResultsQuantity, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, new EmptyFilter(), sortOrder, startPosition);
  }

  public TaskQuery(Filter filter, int expectedResultsQuantity, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
