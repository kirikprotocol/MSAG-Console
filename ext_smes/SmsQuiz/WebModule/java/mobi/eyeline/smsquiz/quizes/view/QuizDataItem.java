package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.*;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class QuizDataItem extends AbstractDataItem {

  public QuizDataItem(QuizData d, String quizId) {
    if((d!=null)&&(quizId!=null)) {
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
      setRepeatQuestion(d.getRepeatQuestion());
      setQuizId(quizId);
      setSourceAddress(d.getSourceAddress());
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

  public void setDateEnd(Date dateEnd) {
    values.put(QuizesDataSource.DATE_END, dateEnd);
  }

  public void setAbFile(String abFile) {
    values.put(QuizesDataSource.AB_FILE, abFile);
  }

  public void setQuestion(String question) {
    values.put(QuizesDataSource.QUESTION, question);
  }

  public void setRepeatQuestion(String repeatQuestion) {
    values.put(QuizesDataSource.REPEAT_QUESTION, repeatQuestion);
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



  public static class State {

    private String name;

    public static final State NEW = new State("NEW");
    public static final State GENERATION = new State("GENERATION");
    public static final State AWAIT = new State("AWAIT");
    public static final State ACTIVE = new State("ACTIVE");
    public static final State FINISHED = new State("FINISHED");
    public static final State FINISHED_ERROR = new State("FINISHED_ERROR");
    public static final State EXPORTING = new State("EXPORTING");
    public static final State UNKNOWN = new State("UNKNOWN");

    private State(String name) {
      this.name = name;
    }

    public String getName() {
      return name;
    }

    public static State getStateByName(String name) {
      if(name.equalsIgnoreCase(NEW.getName())) {
        return NEW;
      }
      if(name.equalsIgnoreCase(GENERATION.getName())) {
        return GENERATION;
      }
      if(name.equalsIgnoreCase(AWAIT.getName())) {
        return AWAIT;
      }
      if(name.equalsIgnoreCase(ACTIVE.getName())) {
        return ACTIVE;
      }
      if(name.equalsIgnoreCase(FINISHED_ERROR.getName())) {
        return FINISHED_ERROR;
      }
      if(name.equalsIgnoreCase(FINISHED.getName())) {
        return FINISHED;
      }
      if(name.equalsIgnoreCase(EXPORTING.getName())) {
        return FINISHED;
      }
      if(name.equalsIgnoreCase(UNKNOWN.getName())) {
        return UNKNOWN;
      }
      return null;
    }
    public static List getStateStringList() {
      LinkedList list = new LinkedList();
      list.add(NEW.getName());
      list.add(GENERATION.getName());
      list.add(AWAIT.getName());
      list.add(ACTIVE.getName());
      list.add(FINISHED.getName());
      list.add(FINISHED_ERROR.getName());
      list.add(EXPORTING.getName());
      return list;
    }
  }
}
