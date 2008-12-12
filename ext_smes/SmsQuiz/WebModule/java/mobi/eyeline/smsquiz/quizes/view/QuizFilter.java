package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Date;

/**
 * author: alkhal
 * Date: Dec 12, 2008
 */
public class QuizFilter implements Filter {

  private Date startDate;

  private Date tillDate;

  private String prefix;

  private String state;

  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    Date quizStartDate = (Date)item.getValue(QuizesDataSource.DATE_BEGIN);
    Date quizEndDate = (Date)item.getValue(QuizesDataSource.DATE_END);
    String quizName = (String)item.getValue(QuizesDataSource.QUIZ_NAME);
    QuizDataItem.State quizState = (QuizDataItem.State)item.getValue(QuizesDataSource.STATE);
    if(startDate!=null && quizStartDate.before(startDate)) {
      return false;
    }
    if(tillDate!=null && quizEndDate.after(tillDate)) {
      return false;
    }
    if(prefix !=null && !quizName.toLowerCase().startsWith(prefix.toLowerCase())) {
      return false;
    }
    if(state!=null && !quizState.getName().equalsIgnoreCase(state)) {
      return false;
    }
    return true;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public String getPrefix() {
    return prefix;
  }

  public void setPrefix(String prefix) {
    this.prefix = prefix;
  }

  public String getState() {
    return state;
  }

  public void setState(String state) {
    this.state = state;
  }
}
