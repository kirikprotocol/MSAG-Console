package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

import java.util.Map;
import java.io.File;

/**
 * author: alkhal
 */
public class QuizManagerTask implements Runnable {

  private static Logger logger = Logger.getLogger(QuizManagerTask.class);


  private Quiz quiz;

  public QuizManagerTask( Quiz quiz) {
    this.quiz = quiz;
  }

  public void run() {
    if(!new File(quiz.getFileName()).exists()) {
      logger.error("Error during creating quiz: quiz's file was deleted "+quiz.getFileName());
      System.out.println("Error during creating quiz: quiz's file was deleted "+quiz.getFileName());
      return;
    }
    quiz.setGenerated(true);
    try {
      quiz.setQuizStatus(Status.QuizStatus.ACTIVE);
    } catch (QuizException e) {
      logger.error(e,e);
    }
    if (logger.isInfoEnabled()) {
      logger.info("Quiz is available now for receiving sms: " + quiz);
    }
  }


}
