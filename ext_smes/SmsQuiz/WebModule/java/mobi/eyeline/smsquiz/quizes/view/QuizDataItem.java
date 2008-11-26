package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.*;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class QuizDataItem extends AbstractDataItem {

  public QuizDataItem() {
  }

  public QuizDataItem(QuizData d, String quizId, String state) {
    if((d!=null)&&(quizId!=null)&&(state!=null)) {
      setAbFile(d.getAbFile());
      setActiveDays(d.getActiveDays());
      setCategories(d.getCategories());
      setDateBegin(d.getDateBegin());
      setDateEnd(d.getDateEnd());
      setDefaultCategory(d.getDefaultCategory());
      setDestAddress(d.getDestAddress());
      setDistrDateEnd(d.getDistrDateEnd());
      setMaxRepeat(d.getMaxRepeat());
      setName(d.getName());
      setQuestion(d.getQuestion());
      setQuizId(quizId);
      setSourceAddress(d.getSourceAddress());
      setState(state);
      setTimeBegin(d.getTimeBegin());
      setTimeEnd(d.getTimeEnd());
      setTxmode(d.getTxmode());
    }
    else {
      throw new IllegalArgumentException("Some argument are null");
    }
  }

  public void setQuizId(String quizId) {
    values.put(QuizesDataSource.QUIZ_ID, quizId);
  }

  public void setDateBegin(Date dateBegin) {
    values.put(QuizesDataSource.DATE_BEGIN, dateBegin);
  }

  public void setState(String state) {
    values.put(QuizesDataSource.STATE, state);
  }

  public void setDateEnd(Date dateEnd) {
    values.put(QuizesDataSource.DATE_END, dateEnd);
  }

  public void setAbFile(String abFile) {
    values.put(QuizesDataSource.AB_FILE, abFile);
  }

  public void setQuestion(String question) {
    values.put(QuizesDataSource.QUESTION, question);
  }

  public void setActiveDays(List activeDays) {
    values.put(QuizesDataSource.ACTIVE_DAYS, activeDays);
  }

  public void setTimeBegin(String timeBegin) {
    values.put(QuizesDataSource.TIME_BEGIN, timeBegin);
  }

  public void setTimeEnd(String timeEnd) {
    values.put(QuizesDataSource.TIME_END, timeEnd);
  }

  public void setTxmode(String txmode) {
    values.put(QuizesDataSource.TX_MODE, txmode);
  }

  public void setSourceAddress(String sourceAddress) {
    values.put(QuizesDataSource.SA, sourceAddress);
  }

  public void setDestAddress(String destAddress) {
    values.put(QuizesDataSource.DA, destAddress);
  }

  public void setMaxRepeat(String maxRepeat) {
    values.put(QuizesDataSource.MAX_REPEAT, maxRepeat);
  }

  public void setDefaultCategory(String defaultCategory) {
    values.put(QuizesDataSource.DEFAULT_CAT, defaultCategory);
  }

  public void setCategories(Collection cat) {
    values.put(QuizesDataSource.CATEGORIES, cat);
  }

  public void setName(String name) {
    values.put(QuizesDataSource.QUIZ_NAME, name);
  }

  public void setDistrDateEnd(Date distrDateEnd) {
    values.put(QuizesDataSource.DISTR_DATE_END, distrDateEnd);
  }

}
