package ru.novosoft.smsc.infosme.backend.tables.retrypolicies;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;

/**
 * User: artem
 * Date: 01.11.2008
 */
public class RetryPolicyQuery extends AbstractQueryImpl {

  public RetryPolicyQuery(int expectedResultsQuantity, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, new EmptyFilter(), sortOrder, startPosition);
  }
}
