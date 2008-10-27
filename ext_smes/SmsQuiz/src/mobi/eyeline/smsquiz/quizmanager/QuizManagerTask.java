package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;

import java.util.Map;

import org.apache.log4j.Logger;

/**
 * author: alkhal
 */
public class QuizManagerTask implements Runnable {

  private static Logger logger = Logger.getLogger(QuizManagerTask.class);

  private Map<String, Quiz> quizMap;

  private Quiz quiz;

  public QuizManagerTask(Map<String, Quiz> quizMap, Quiz quiz) {
    this.quiz = quiz;
    this.quizMap = quizMap;
  }

  public void run() {
    quizMap.put(quiz.getDestAddress(), quiz);
    if (logger.isInfoEnabled()) {
      logger.info("Quiz is available now for receiving sms: " + quiz);
    }
  }


}
