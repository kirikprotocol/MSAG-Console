package ru.novosoft.smsc.infosme.backend.tables.schedules;

import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 1:19:58 PM
 */
public class ScheduleQuery extends AbstractQueryImpl {
  public ScheduleQuery(int expectedResultsQuantity, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, new EmptyFilter(), sortOrder, startPosition);
  }
}
