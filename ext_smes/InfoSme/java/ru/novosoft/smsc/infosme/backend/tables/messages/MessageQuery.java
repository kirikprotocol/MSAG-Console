package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.Vector;

/**
 * User: artem
 * Date: 29.05.2008
 */

public class MessageQuery extends AbstractQueryImpl {
  public MessageQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }

  public MessageQuery(int expectedResultsQuantity, Filter filter, Vector sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
