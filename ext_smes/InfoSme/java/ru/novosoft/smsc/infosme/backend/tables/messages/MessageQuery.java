package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2004
 * Time: 20:48:32
 * To change this template use Options | File Templates.
 */
public class MessageQuery extends AbstractQueryImpl
{
  public MessageQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
