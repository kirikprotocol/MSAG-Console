package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizQuery extends AbstractQueryImpl {
  public QuizQuery(int expectedResultsQuantity, Filter filter, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
