package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.util.List;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import mobi.eyeline.smsquiz.SmsQuizContext;
import mobi.eyeline.smsquiz.QuizesDataSource;
import mobi.eyeline.smsquiz.beans.util.Tokenizer;

import javax.servlet.http.HttpServletRequest;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class SmsQuizBean extends PageBean {


  public static final int RESULT_OPTIONS = PageBean.PRIVATE_RESULT;
  public static final int RESULT_QUIZES = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_REPLYSTATS = PageBean.PRIVATE_RESULT + 2;
  public static final int RESULT_RESULTS = PageBean.PRIVATE_RESULT + 3;
  public static final int RESULT_DISTR = PageBean.PRIVATE_RESULT + 4;
  public static final int RESULT_MAIN = PageBean.PRIVATE_RESULT + 5;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 6;

  protected SmsQuizContext smsQuizContext;
  protected Config config;

  protected String smeId = "SmsQuiz";
  protected boolean smeRunning = false;

  protected String mbMenu = null;


  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      smsQuizContext = SmsQuizContext.getInstance(appContext, smeId);
      config = smsQuizContext.getConfig();
    } catch (Throwable e) {
      logger.error("Couldn't get Quiz config", e);
      return error("infosme.error.config_load", e);
    }

    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;
    if ("quizes".equals(mbMenu)) result = RESULT_QUIZES;
    else if ("options".equals(mbMenu)) result = RESULT_OPTIONS;
    else if ("reply_stats".equals(mbMenu)) result = RESULT_REPLYSTATS;
    else if ("quiz_result".equals(mbMenu)) result = RESULT_RESULTS;
    else if ("distr".equals(mbMenu)) result = RESULT_DISTR;
    else if ("main".equals(mbMenu)) result = RESULT_MAIN;

    mbMenu = null;
    return result;
  }


  protected Config getConfig() {
    return config;
  }

  public SmsQuizContext getSmsQuizContext() {
    return smsQuizContext;
  }

  public String getSmeId() {
    return smeId;
  }

  public String getMbMenu() {
    return mbMenu;
  }

  public void setMbMenu(String mbMenu) {
    this.mbMenu = mbMenu;
  }

  protected Date getActualStartDate(String quizId) {
    Date date = null;
    String info;
    SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");
    try {
      info = smsQuizContext.getSmsQuiz().getStatus(quizId);
    } catch (AdminException e) {
      logger.warn(e,e);
      try{
        if(logger.isInfoEnabled()){
          logger.error("Trying to refresh quiz with id: "+quizId);
        }
        QuizesDataSource.getInstance().refreshQuiz(quizId);
      } catch(Exception ex) {
        logger.error(e,e);
        e.printStackTrace();
      }
      return null;
    }
    if(info.equals("")) {
      return null;
    }
    Tokenizer tokenizer = new Tokenizer(info,"|");
    tokenizer.next();tokenizer.next();tokenizer.next();
    String dateStr = tokenizer.next();
    try {
      date = df.parse(dateStr);
    } catch (ParseException e) {
      logger.error(e,e);
    }

    return date;
  }

  protected String getTaskId(String quizId) {
    String id = null;
    try {
      String distrId = smsQuizContext.getSmsQuiz().getDistrId(quizId);
      if(!distrId.equals("")) {
        id = distrId;
      }
    } catch (AdminException e) {
      logger.warn(e,e);
      try{
        if(logger.isInfoEnabled()){
          logger.error("Trying to refresh quiz with id: "+quizId);
        }
        QuizesDataSource.getInstance().refreshQuiz(quizId);
      } catch(Exception ex) {
        logger.error(e,e);
        e.printStackTrace();
      }
    }
    return id;
  }

}
