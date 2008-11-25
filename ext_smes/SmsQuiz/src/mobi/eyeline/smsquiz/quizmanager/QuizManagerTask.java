package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.io.File;

/**
 * author: alkhal
 */
public class QuizManagerTask implements Runnable {

  private static Logger logger = Logger.getLogger(QuizManagerTask.class);


  private Quiz quiz;
  private ConcurrentHashMap<String, Quiz> qInternal;
  private ConcurrentHashMap<String, Quiz> quizesMap;

  public QuizManagerTask(Quiz quiz, ConcurrentHashMap<String, Quiz> qInternal, ConcurrentHashMap<String, Quiz> quizesMap) {
    this.quiz = quiz;
    this.qInternal = qInternal;
    this.quizesMap = quizesMap;
  }

  public void run() {
    if(!new File(quiz.getFileName()).exists()) {
      logger.error("Error during creating quiz: quiz's file was deleted "+quiz.getFileName());
      System.out.println("Error during creating quiz: quiz's file was deleted "+quiz.getFileName());
      return;
    }
    quiz.setGenerated(true);
    try{
      quiz.setQuizStatus(Status.QuizStatus.AWAIT);
    }catch (QuizException e){}
    if (logger.isInfoEnabled()) {
      logger.info("Quiz is available now for receiving sms: " + quiz);
    }
    QuizCreator quizCreator = new QuizCreator(quiz);
    if(quiz.isActive()) {
      quizCreator.start();
    } else {
      long delay = quiz.getDateBegin().getTime() -System.currentTimeMillis();
      ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable r) {
          return new Thread(r,"QuizCreator: "+quiz.getQuizName());
        }
      });
      executor.schedule(quizCreator,delay,java.util.concurrent.TimeUnit.MILLISECONDS);
    }
  }

  private class QuizCreator extends Thread{

    private Quiz quiz;

    public QuizCreator(Quiz quiz) {
      this.quiz = quiz;

    }
    public void run() {
      try{
        if(qInternal.containsValue(quiz)) {
          quizesMap.put(quiz.getDestAddress(),quiz);
          if(logger.isInfoEnabled()) {
            logger.info("Quiz added to map: "+quiz);
          }
          quiz.setQuizStatus(Status.QuizStatus.ACTIVE);
        }
      }catch (Exception e) {
        logger.error(e,e);
      }
    }
  }


}
