package mobi.eyeline.smsquiz.quizmanager.quiz;

import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * author: alkhal
 */

public class StatusFile {


  private static final Logger logger = Logger.getLogger(StatusFile.class);

  private String statusFileName;

  private static final String DISTR_ID = "distribution.id";
  private static final String QUIZ_ST = "quiz.status";
  private static final String ERROR_REASON = "quiz.error.reason";
  private static final String ERROR_CODE = "quiz.error.id";
  private static final String ACTUAL_START_DATE = "quiz.start";

  private final static SimpleDateFormat DF = new SimpleDateFormat("yyyyMMddHHmmss");

  private Properties prop;

  private Lock writeLock = new ReentrantLock();

  @SuppressWarnings({"EmptyCatchBlock", "ResultOfMethodCallIgnored"})
  StatusFile(String statusFileName) throws QuizException {
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
      String tmp;
      if ((tmp = prop.getProperty(DISTR_ID)) != null) {  // if property exist => distribution already created
        prop.setProperty(DISTR_ID, tmp);
      }
      prop.setProperty(QUIZ_ST, Quiz.Status.NEW.toString());
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

  synchronized String getDistrId() {
    return prop.getProperty(DISTR_ID);
  }

  synchronized void setDistrId(String ident) throws QuizException {
    prop.setProperty(DISTR_ID, ident);
    storeProps();
  }

  synchronized void setQuizStatus(Quiz.Status quizStatus) throws QuizException {
    prop.setProperty(QUIZ_ST, quizStatus.toString());
    storeProps();
  }

  synchronized void setQuizErrorStatus(QuizError error, String reason) throws QuizException {
    prop.setProperty(QUIZ_ST, Quiz.Status.FINISHED_ERROR.toString());
    prop.setProperty(ERROR_CODE, error.getCode());
    prop.setProperty(ERROR_REASON, reason);
    storeProps();
  }

  synchronized Quiz.Status getQuizStatus() {
    return (prop.getProperty(QUIZ_ST) == null) ? null : Quiz.Status.valueOf(prop.getProperty(QUIZ_ST));
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  synchronized private void storeProps() throws QuizException {
    OutputStream outputStream = null;
    try {
      writeLock.lock();
      outputStream = new FileOutputStream(statusFileName);
      prop.store(outputStream, "");
    } catch (Exception e) {
      logger.error(e, e);
      throw new QuizException(e.toString(), e);
    } finally {
      if (outputStream != null) {
        try {
          outputStream.close();
        } catch (Exception e) {
        }
      }
      writeLock.unlock();
    }
  }

  synchronized String getStatusFileName() {
    return statusFileName;
  }

  synchronized void setActualStartDate(Date date) throws QuizException {
    if (date == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    prop.setProperty(ACTUAL_START_DATE, DF.format(date));
    storeProps();
  }

  synchronized Date getActualStartDate() throws QuizException {
    try {
      if (prop.getProperty(ACTUAL_START_DATE) == null) {
        return null;
      }
      return DF.parse(prop.getProperty(ACTUAL_START_DATE));
    } catch (ParseException e) {
      logger.error(e, e);
      throw new QuizException(e);
    }
  }

  synchronized String getActualStartDateStr() {
    return prop.getProperty(ACTUAL_START_DATE);
  }

  synchronized String getErrorCode() {
    return prop.getProperty(ERROR_CODE);
  }

  synchronized String getErrorReason() {
    return prop.getProperty(ERROR_REASON);
  }

}
