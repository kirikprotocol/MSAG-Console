package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.DirListener;

import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.Date;

import org.apache.log4j.Logger;

class QuizCollector implements Runnable {

  private static final Logger logger = Logger.getLogger(QuizCollector.class);

  private final ConcurrentHashMap<String, Quiz> quizesMap;
  private final DirListener dirListener;

  public QuizCollector(ConcurrentHashMap<String, Quiz> quizesMap, DirListener dirListener) {
    this.quizesMap = quizesMap;
    this.dirListener = dirListener;
  }

  public void run() {
    logger.info("QuizCollectors starts...");
    for (Map.Entry<String, Quiz> entry : quizesMap.entrySet()) {
      Quiz quiz = entry.getValue();
      if (quiz.getDateEnd().before(new Date())) {
        try {
          dirListener.remove(quiz.getFileName(), true);
        } catch (QuizException e) {
          logger.error("Error removing file from DirListener");
        }
        try {
          quiz.exportStats();
          quizesMap.remove(entry.getKey());
        } catch (QuizException e) {
          logger.error("Error creating resultStatistics");
        }
        if (logger.isInfoEnabled()) {
          logger.info("QuizCollector removed quiz: " + quiz);
        }

      }
    }
    logger.info("QuizCollectors finished");

  }


}
 
