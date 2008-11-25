package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

class QuizCollector implements Runnable {

  private static final Logger logger = Logger.getLogger(QuizCollector.class);

  private final ConcurrentHashMap<String, Quiz> quizesMap;
  private final ConcurrentHashMap<String, Quiz> qInternal;

  public QuizCollector(ConcurrentHashMap<String, Quiz> quizesMap, ConcurrentHashMap<String, Quiz> qInternal) {
    this.quizesMap = quizesMap;
    this.qInternal = qInternal;
  }

  public void run() {
    logger.info("QuizCollectors starts...");
    try {
      for (Map.Entry<String, Quiz> entry : qInternal.entrySet()) {
        Quiz quiz = entry.getValue();
        Date now = new Date();
        if (quiz.getDateEnd().before(now)) {
          try {
            quizesMap.remove(entry.getKey());
            qInternal.remove(entry.getKey());
            quiz.exportStats();
            quiz.shutdown();
            quiz.setQuizStatus(Status.QuizStatus.FINISHED);
          } catch (QuizException e) {
            logger.error("Error creating resultStatistics");
          }
          if (logger.isInfoEnabled()) {
            logger.info("QuizCollector removed quiz: " + quiz);
          }
        }
        else {
          if((!quiz.getQuizStatus().equals(Status.QuizStatus.ACTIVE))&&(quiz.isGenerated())&&(now.after(quiz.getDateBegin()))) {
            quiz.setQuizStatus(Status.QuizStatus.ACTIVE);
          }
        }
      }
    } catch (Throwable e) {
      logger.error(e);
      e.printStackTrace();
    }
    logger.info("QuizCollectors finished");

  }


}
 
