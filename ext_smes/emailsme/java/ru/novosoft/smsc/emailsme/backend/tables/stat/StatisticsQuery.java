package ru.novosoft.smsc.emailsme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * User: artem
 * Date: 04.09.2008
 */

public class StatisticsQuery extends AbstractQueryImpl {
  public StatisticsQuery(StatisticsFilter filter) {
    super(0, filter, "", 0);
  }
}
