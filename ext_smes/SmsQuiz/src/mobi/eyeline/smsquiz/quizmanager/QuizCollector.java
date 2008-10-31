package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.dirlistener.DirListener;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

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
    try {
      for (Map.Entry<String, Quiz> entry : quizesMap.entrySet()) {
        Quiz quiz = entry.getValue();
        if (quiz.getDateEnd().before(new Date())) {
          try {
            dirListener.remove(quiz.getFileName(), true);
            String statusFile = quiz.getStatusFileName();
            if(statusFile!=null) {
              dirListener.remove(quiz.getStatusFileName(), true);
            }
          } catch (QuizException e) {
            logger.error("Error removing file from DirListener");
          }
          try {
            quizesMap.remove(entry.getKey());
            quiz.exportStats();
            quiz.shutdown();
          } catch (QuizException e) {
            logger.error("Error creating resultStatistics");
          }
          if (logger.isInfoEnabled()) {
            logger.info("QuizCollector removed quiz: " + quiz);
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
 
