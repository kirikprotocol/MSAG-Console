package ru.novosoft.smsc.jsp.util.tables.impl.regions;

import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionsQuery extends AbstractQueryImpl {
  public RegionsQuery(int expectedResultsQuantity, String sortOrder, int startPosition, RegionsFilter filter) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
