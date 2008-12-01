package mobi.eyeline.smsquiz.quizmanager.quiz;

import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;

/**
 * author: alkhal
 */

public class Status {


  private static final Logger logger = Logger.getLogger(Status.class);

  private String ident;
  private String statusFileName;
  private QuizStatus quizStatus;

  public static enum QuizStatus {
    NEW, AWAIT, GENERATION, FINISHED,
    FINISHED_ERROR, ACTIVE
  }

  public static final String DISTR_ID = "distribution.id";
  private static final String QUIZ_ST = "quiz.status";
  private static final String ERROR_REASON = "quiz.error.reason";
  private static final String ERROR_CODE = "quiz.error.id";
  private static final String ACTUAL_START_DATE = "quiz.start";

  private final static SimpleDateFormat DF = new SimpleDateFormat("yyyyMMddHHmmss");

  private Properties prop;

  @SuppressWarnings({"EmptyCatchBlock"})
  Status(String statusFileName) throws QuizException {
    this.statusFileName = statusFileName;
    File stFile = new File(statusFileName);
    File parentFile = stFile.getParentFile();
    if ((parentFile != null) && (!parentFile.exists())) {
      parentFile.mkdirs();
    }
    InputStream inputStream = null;
    try {
      prop = new Properties();
      if (!stFile.exists()) {
        stFile.createNewFile();
      }
      inputStream = new FileInputStream(stFile);
      prop = new Properties();
      prop.load(inputStream);
      String ident;
      if ((ident = prop.getProperty(DISTR_ID)) != null) {  // if property exist => distribution already created
        this.ident = ident;
      }
      setQuizStatus(QuizStatus.NEW);
    } catch (IOException e) {
      logger.error(e, e);
      throw new QuizException(e.toString(), e);
    } finally {
      if (inputStream != null) {
        try {
          inputStream.close();
        } catch (IOException e) {
        }
      }
    }
  }

  String getId() {
    return ident;
  }

  void setId(String ident) throws QuizException {
    this.ident = ident;
    prop.setProperty(DISTR_ID, ident);
    storeProps();
  }

  void setQuizStatus(QuizStatus quizStatus) throws QuizException {
    this.quizStatus = quizStatus;
    prop.setProperty(QUIZ_ST, quizStatus.toString());
    storeProps();
  }

  void setQuizErrorStatus(QuizError error, String reason) throws QuizException {
    quizStatus = QuizStatus.FINISHED_ERROR;
    prop.setProperty(QUIZ_ST, quizStatus.toString());
    prop.setProperty(ERROR_CODE, error.getCode());
    prop.setProperty(ERROR_REASON, reason);
    storeProps();
  }

  QuizStatus getQuizStatus() {
    return quizStatus;
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  private void storeProps() throws QuizException {
    OutputStream outputStream = null;
    try {
      outputStream = new FileOutputStream(statusFileName);
      prop.store(outputStream, "");
    } catch (IOException e) {
      logger.error(e, e);
      throw new QuizException(e.toString(), e);
    } finally {
      if (outputStream != null) {
        try {
          outputStream.close();
        } catch (IOException e) {
        }
      }
    }
  }

  String getStatusFileName() {
    return statusFileName;
  }

  void setActualStartDate(Date date) throws QuizException {
    if((prop.getProperty(ACTUAL_START_DATE)==null)||(prop.getProperty(ACTUAL_START_DATE).equals(""))) {
      Date now = new Date();
      if (now.before(date)) {
        prop.setProperty(ACTUAL_START_DATE, DF.format(date));
      } else {
        prop.setProperty(ACTUAL_START_DATE, DF.format(now));
      }
      storeProps();
    }
  }

  Date getActualStartDate() throws QuizException {
    Date date;
    try {
      date = DF.parse(prop.getProperty(ACTUAL_START_DATE));
    } catch (ParseException e) {
      logger.error(e, e);
      throw new QuizException(e);
    }
    return date;
  }

}
