package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizQuery extends AbstractQueryImpl {
  public QuizQuery(int expectedResultsQuantity, String sortOrder, int startPosition) {
    super(expectedResultsQuantity, new EmptyFilter(), sortOrder, startPosition);
  }
}
