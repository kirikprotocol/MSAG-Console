package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Date;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class QuizDataItem extends AbstractDataItem {

  public QuizDataItem(String quizId, String quizName, Date dateBegin, Date dateEnd, String state) {
    values.put(QuizesDataSource.QUIZ_ID, quizId);
    values.put(QuizesDataSource.QUIZ_NAME, quizName);
    values.put(QuizesDataSource.DATE_BEGIN, dateBegin);
    values.put(QuizesDataSource.DATE_END, dateEnd);
    values.put(QuizesDataSource.STATE, state);
  }

  public String getQuizId() {
    return (String) values.get(QuizesDataSource.QUIZ_ID);
  }

  public String getQuizName() {
    return (String) values.get(QuizesDataSource.QUIZ_NAME);
  }

  public Date getDateBegin() {
    return (Date) values.get(QuizesDataSource.DATE_BEGIN);
  }

  public Date getDateEnd() {
    return (Date) values.get(QuizesDataSource.DATE_END);
  }

  public String getState() {
    return (String) values.get(QuizesDataSource.STATE);
  }

}
