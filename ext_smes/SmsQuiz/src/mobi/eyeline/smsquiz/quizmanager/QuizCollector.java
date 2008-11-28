package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.LinkedList;
import java.util.concurrent.ConcurrentHashMap;

class QuizCollector implements Runnable {

  private static final Logger logger = Logger.getLogger(QuizCollector.class);

  private final ConcurrentHashMap<String, Quiz> quizesMap;
  private final LinkedList<Quiz> qInternal;

  public QuizCollector(ConcurrentHashMap<String, Quiz> quizesMap, LinkedList<Quiz> qInternal) {
    this.quizesMap = quizesMap;
    this.qInternal = qInternal;
  }

  public void run() {
    logger.info("QuizCollectors starts...");
    try {
      for (Quiz quiz : qInternal) {
        Date now = new Date();
        if ((quiz.getDateEnd().before(now)) && (!quiz.isExported())) {
          try {
            quizesMap.remove(quiz.getDestAddress());
            quiz.exportStats();
            quiz.shutdown();
            quiz.setQuizStatus(Status.QuizStatus.FINISHED);
          } catch (QuizException e) {
            logger.error("Error creating resultStatistics");
          }
          if (logger.isInfoEnabled()) {
            logger.info("QuizCollector removed quiz: " + quiz);
          }
        } else {
          if ((quiz.getQuizStatus().equals(Status.QuizStatus.GENERATION)
              ||quiz.getQuizStatus().equals(Status.QuizStatus.AWAIT))
              && (quiz.isGenerated()) && (quiz.isActive())) {
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
 
