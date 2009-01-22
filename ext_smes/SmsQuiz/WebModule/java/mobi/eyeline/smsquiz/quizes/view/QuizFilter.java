package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.admin.AdminException;

import java.util.Date;
import java.util.StringTokenizer;

import mobi.eyeline.smsquiz.SmsQuizService;
import mobi.eyeline.smsquiz.quizes.QuizState;
import mobi.eyeline.smsquiz.QuizesDataSource;
import org.apache.log4j.Category;

/**
 * author: alkhal
 * Date: Dec 12, 2008
 */
public class QuizFilter implements Filter {

  private Date startDate;

  private Date tillDate;

  private String prefix;

  private String state;

  private SmsQuizService smsQuizService;

  private static final Category logger = Category.getInstance(QuizFilter.class);

  public boolean isEmpty() {
    return false;
  }

  public QuizFilter(SmsQuizService smsQuizService) {
    if(smsQuizService ==null) {
      logger.error("Some argument are null");
      throw new IllegalArgumentException("Some argument are null");
    }
    this.smsQuizService = smsQuizService;
  }

  public boolean isItemAllowed(DataItem item) {
    Date quizStartDate = (Date)item.getValue(QuizesDataSource.DATE_BEGIN);
    Date quizEndDate = (Date)item.getValue(QuizesDataSource.DATE_END);
    String quizName = (String)item.getValue(QuizesDataSource.QUIZ_NAME);
    String quizId = (String) item.getValue(QuizesDataSource.QUIZ_ID);

    if(startDate!=null && quizStartDate.before(startDate)) {
      return false;
    }
    if(tillDate!=null && quizEndDate.after(tillDate)) {
      return false;
    }
    if(prefix !=null && !quizName.toLowerCase().startsWith(prefix.toLowerCase())) {
      return false;
    }
    if(state!=null && !getStatus(quizId).equalsIgnoreCase(state)) {
      return false;
    }
    return true;
  }

  private String getStatus(String quizId) {
    String info;
    try {
      info = smsQuizService.getStatus(quizId);
    } catch (AdminException e) {
      logger.error(e,e);
      return QuizState.UNKNOWN.getName();
    }
    if(info.equals("")) {
      return QuizState.UNKNOWN.getName();
    }
    else {
      return new StringTokenizer(info,"|").nextToken();
    }
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
