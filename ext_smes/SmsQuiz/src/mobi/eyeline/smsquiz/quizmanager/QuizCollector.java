package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.LinkedList;
import java.util.concurrent.*;

import com.eyeline.utils.ThreadFactoryWithCounter;

class QuizCollector {

  private static final Logger logger = Logger.getLogger(QuizCollector.class);

  private final ConcurrentHashMap<String, Quiz> quizesMap;
  private final Quizes quizes;

  private ScheduledExecutorService executor;

  public QuizCollector(ConcurrentHashMap<String, Quiz> quizesMap, Quizes quizes) {
    this.quizesMap = quizesMap;
    this.quizes = quizes;

  }

  public void start() {
    executor = Executors.newScheduledThreadPool(10, new ThreadFactoryWithCounter("QuizCollector-"));
  }

  public void shutdown() {
    executor.shutdownNow();
  }

  public void alert() throws QuizException{
    long time = calcTime().getTime();
    if (time == Long.MAX_VALUE)
      return;

    time = time - System.currentTimeMillis();
    if (time < 0)
      time = 0;
    try{
      executor.schedule(new Task(), time , TimeUnit.MILLISECONDS);
    }catch (RejectedExecutionException e) {
      logger.error(e,e);
      throw new QuizException(e);
    }
    if(logger.isInfoEnabled()) {
      logger.info("Task will be run after: "+time+" ms");
    }
  }

  private Date calcTime() throws QuizException{
    VisitorForCalc visitor = new VisitorForCalc();
    try {
      quizes.visit(visitor);
      Date minStart = visitor.getMinStart();
      Date minEnd = visitor.getMinEnd();
      return (minStart.before(minEnd)) ? minStart : minEnd;
    } catch (QuizException e) {
      logger.error(e,e);
      throw e;
    }
  }


  private class Task implements Runnable {
    public void run() {
      logger.info("QuizCollector starts...");
      try {
        Quizes.Visitor visitor = new VisitorForTask();
        quizes.visit(visitor);
        alert();
      } catch (Throwable e) {
        logger.error(e);
        e.printStackTrace();
      }
      logger.info("QuizCollector finished");

    }
  }

  private class VisitorForCalc implements Quizes.Visitor{
      private Date minStart;
      private Date minEnd;
      VisitorForCalc() {
        this.minStart = new Date(Long.MAX_VALUE);
        this.minEnd = new Date(Long.MAX_VALUE);
      }
      public void visit(Quiz quiz) {
        final Date now = new Date();
        if ((!quizesMap.containsValue(quiz)) && (!quiz.isExported()) && quiz.getDateBegin().before(minStart))   {
          minStart = quiz.getDateBegin();
        }
        if (quiz.getDateEnd().after(now) && (!quiz.isExported()) && quiz.getDateEnd().before(minEnd))  {
          minEnd = quiz.getDateEnd();
        }
      }
      public Date getMinEnd() {
        return minEnd;
      }
      public Date getMinStart() {
        return minStart;
      }
  }

  private class VisitorForTask implements Quizes.Visitor{

    Date now = new Date();

    public void visit(final Quiz quiz) throws QuizException{
      if ((quiz.getDateEnd().before(now))) {
        if (quizesMap.get(quiz.getDestAddress()) == quiz) {
          quizesMap.remove(quiz.getDestAddress());
          if (logger.isInfoEnabled()) {
            logger.info("QuizCollector removed quiz from map: " + quiz);
          }
        }
        executor.execute(new Runnable() {
          public void run() {
            try {
              quiz.exportStats();
              quiz.shutdown();
              quiz.setQuizStatus(Status.QuizStatus.FINISHED);
            } catch (QuizException e) {
              logger.error("Error creating result statistics");
            }
          }
        });
      } else {
        quizesMap.put(quiz.getDestAddress(), quiz);
        quiz.setQuizStatus(Status.QuizStatus.ACTIVE);
        if (logger.isInfoEnabled()) {
          logger.info("QuizCollector added quiz into map: " + quiz);
        }
      }
    }
  }


}
 
