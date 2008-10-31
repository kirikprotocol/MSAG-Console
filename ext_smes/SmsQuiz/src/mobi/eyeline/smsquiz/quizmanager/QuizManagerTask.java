package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import org.apache.log4j.Logger;

import java.util.Map;

/**
 * author: alkhal
 */
public class QuizManagerTask implements Runnable {

  private static Logger logger = Logger.getLogger(QuizManagerTask.class);

  private Quiz quiz;

  public QuizManagerTask(Quiz quiz) {
    this.quiz = quiz;
  }

  public void run() {
    quiz.setGenerated(true);
    if (logger.isInfoEnabled()) {
      logger.info("Quiz is available now for receiving sms: " + quiz);
    }
  }


}
