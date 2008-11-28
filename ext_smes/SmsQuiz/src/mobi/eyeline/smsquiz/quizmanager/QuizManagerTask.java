package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

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
    try {
      if (quiz.getQuizStatus().equals(Status.QuizStatus.GENERATION)) {
        if(!quiz.isActive()) {
          quiz.setQuizStatus(Status.QuizStatus.AWAIT);
        }
        else {
          quiz.setQuizStatus(Status.QuizStatus.ACTIVE);
        }
      }
    } catch (QuizException e) {
      logger.error(e, e);
    }
    if (logger.isInfoEnabled()) {
      logger.info("Quiz is generated: " + quiz);
    }
  }

}
