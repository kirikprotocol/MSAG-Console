package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * User: artem
 * Date: 02.06.2008
 */

public class StatisticsQuery extends AbstractQueryImpl {
  public StatisticsQuery(Filter filter) {
    super(1000, filter, "", 0);
  }
}
