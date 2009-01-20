package mobi.eyeline.smsquiz.quizmanager;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizError;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.concurrent.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

@SuppressWarnings({"EmptyCatchBlock"})
public class QuizCollector {

  private static final Logger logger = Logger.getLogger(QuizCollector.class);

  private final ConcurrentHashMap<String, Quiz> quizesMap;
  private final Quizes quizes;
  private final DistributionManager dm;

  private ScheduledExecutorService scheduler;
  private ExecutorService executor;
  private ScheduledFuture currentTask;

  private final Lock scheduleLock = new ReentrantLock();

  private final long checkTimeot = 15000;

  public QuizCollector(ConcurrentHashMap<String, Quiz> quizesMap, Quizes quizes,
                       DistributionManager dm) {
    this.quizesMap = quizesMap;
    this.quizes = quizes;
    this.dm = dm;
  }

  public void start() {
    scheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "QuizCollector");
      }
    });
    executor = new ThreadPoolExecutor(1, 10, 60000, TimeUnit.MILLISECONDS, new LinkedBlockingQueue<Runnable>(),
        new ThreadFactoryWithCounter("QuizCollectorExecutor-"));
  }

  public void shutdown() {
    scheduler.shutdownNow();
    executor.shutdownNow();
  }

  private void updateSchedule(long newStartTime) throws QuizException {
    try {
      scheduleLock.lock();
      if (newStartTime != Long.MAX_VALUE) {
        if (currentTask != null) { // Check we need to cancel current task
          currentTask.cancel(false); // Cancel current task
        }
        long newTaskDelay = Math.max(0, newStartTime - System.currentTimeMillis()); // delay for new task
        currentTask = scheduler.schedule(new Task(), newTaskDelay, TimeUnit.MILLISECONDS);
        if (logger.isInfoEnabled())
          logger.info("Task will be run at: " + new Date(newStartTime));
      }
    } catch (Throwable e) {
      logger.error(e, e);
      throw new QuizException(e);
    } finally {
      scheduleLock.unlock();
    }
  }

  public void alert() throws QuizException {
    try {
      if (logger.isInfoEnabled()) {
        logger.info("Alert executed");
      }
      updateSchedule(calcMinStartTime());
    } catch (Throwable e) {
      logger.error(e, e);
      throw new QuizException(e);
    }
  }

  private long calcMinStartTime(Quiz quiz) {
    if (quiz.getDateBegin() != null && quiz.getDateEnd() != null) {
      switch (quiz.getQuizStatus()) {
        case NEW:
          if (!quiz.isDistrGenerated()) return quiz.getDateBegin().getTime();
          break;
        case GENERATION:
          return quiz.getLastDistrStatusCheck() + checkTimeot;
        case AWAIT:
          return quiz.getDateBegin().getTime();
        case ACTIVE:
          return quiz.getDateEnd().getTime();
        case FINISHED:
          if (!quiz.isExported()) return System.currentTimeMillis();
          break;
        case FINISHED_ERROR:
          if ((quiz.getDestAddress() != null) && (quizesMap.get(quiz.getDestAddress()) == quiz))
            return System.currentTimeMillis();
          break;
      }
    }
    return Long.MAX_VALUE;
  }

  private long calcMinStartTime() throws QuizException {
    final long[] minDate = new long[]{Long.MAX_VALUE};
    try {
      quizes.visit(new Quizes.Visitor() {
        public void visit(Quiz quiz) throws QuizException {
          long time = calcMinStartTime(quiz);
          if (time < minDate[0])
            minDate[0] = time;
        }
      });
      return minDate[0];
    } catch (QuizException e) {
      logger.error(e, e);
      throw e;
    }
  }


  private class Task implements Runnable {
    public void run() {
      logger.info("QuizCollector starts...");
      try {
        Quizes.Visitor visitor = new VisitorForTask();
        quizes.visit(visitor);       //already synchronized
      } catch (Throwable e) {
        logger.error(e, e);
        e.printStackTrace();
      } finally {
        try {
          alert();
        } catch (QuizException e) {
          logger.error(e, e);
        }
      }
      logger.info("QuizCollector finished");
    }
  }

  private class VisitorForTask implements Quizes.Visitor {

    final long now = System.currentTimeMillis();

    public void visit(final Quiz quiz) throws QuizException {
      if (quiz.getQuizStatus() == Quiz.Status.ACTIVE) { // If quiz time expired
        if (quiz.getDateEnd().getTime() <= now) {
          if (logger.isInfoEnabled())
            logger.info("Active quiz with expired end date was found and will be finished: " + quiz);
          setFinished(quiz);
        }

      } else if (quiz.getQuizStatus() == Quiz.Status.FINISHED) { // Export stats if needed
        if (!quiz.isExported()) {
          if (logger.isInfoEnabled())
            logger.info("Finished quiz was found, execute task to export it's results: " + quiz);
          exportStats(quiz);
        }
      } else if ((quiz.getQuizStatus() == Quiz.Status.FINISHED_ERROR)
          && (quiz.getDestAddress() != null) && (quizesMap.get(quiz.getDestAddress()) == quiz)) { // Quiz failed
        if (logger.isInfoEnabled())
          logger.info("Failed quiz was found and will be deleted from dest addr map: " + quiz);
        deleteFromMap(quiz);

      } else if (quiz.getQuizStatus() == Quiz.Status.EXPORTING) {
        // do nothing
      } else {

        if (quiz.getDateEnd().getTime() > now) {

          if (quiz.getQuizStatus() == Quiz.Status.AWAIT && quiz.getDateBegin().getTime() <= now) { // Quiz have to start
            if (logger.isInfoEnabled())
              logger.info("Await quiz with expired start date was found and will be activated: " + quiz);
            setActive(quiz);

          } else if (quiz.getQuizStatus() == Quiz.Status.GENERATION) { // Check distribution status
            if (logger.isInfoEnabled())
              logger.info("Generated quiz was found, it generation state will be checked: " + quiz);
            checkState(quiz);

          } else if (quiz.getQuizStatus() == Quiz.Status.NEW) {   // Create distribution
            if (!quiz.isDistrGenerated()) {
              if (logger.isInfoEnabled())
                logger.info("New quiz was found, create distr for it: " + quiz);
              createDistribution(quiz);
            } else {
              quiz.setQuizStatus(Quiz.Status.AWAIT);
            }
          }
        } else {
          if (logger.isInfoEnabled())
            logger.warn("Found quiz in state " + quiz.getQuizStatus() + " and expired end date was found:" + quiz);
          try {
            if (logger.isInfoEnabled()) {
              logger.warn("Trying to export stats for it: ");
              exportStats(quiz);
            }
          } catch (Exception e) {
            logger.error(e, e);
            throw new QuizException(e);
          }

        }
      }
    }
  }

  private void createDistribution(final Quiz quiz) throws QuizException {
    try {
      if (!quiz.isDistrGenerated()) {
        executor.execute(new Runnable() {
          public void run() {
            try {
              if (!quiz.isDistrGenerated()) {
                quiz.createDistribution();      //already synchronized
                quiz.setLastDistrStatusCheck(System.currentTimeMillis());
                quiz.setQuizStatus(Quiz.Status.GENERATION);
              }
            } catch (Throwable e) {
              logger.error(e, e);
            }
          }
        });
      }
    } catch (Throwable e) {
      logger.error(e, e);
      throw new QuizException(e.toString(), e);
    }
  }


  private void setActive(Quiz quiz) throws QuizException {
    quizesMap.put(quiz.getDestAddress(), quiz);
    try {
      quiz.setQuizStatus(Quiz.Status.ACTIVE);
    } catch (Exception e) {
      logger.error(e, e);
      throw new QuizException(e);
    }
    if (logger.isInfoEnabled())
      logger.info("QuizCollector added quiz into map: " + quiz);
  }

  private void setFinished(Quiz quiz) throws QuizException {
    deleteFromMap(quiz);
    exportStats(quiz);
  }

  private void deleteFromMap(Quiz quiz) {
    if (quizesMap.get(quiz.getDestAddress()) == quiz) {
      quizesMap.remove(quiz.getDestAddress());
      if (logger.isInfoEnabled())
        logger.info("QuizCollector removed quiz from map: " + quiz);
    }
  }

  private void checkState(Quiz quiz) throws QuizException {
    if (quiz.getLastDistrStatusCheck() + checkTimeot > System.currentTimeMillis())
      return;
    String id = quiz.getDistrId();
    if (id == null) {
      logger.error("Distr id is null for quiz: " + quiz);
      return;
    }
    try {
      DistributionManager.State state = dm.getState(id);
      if (state.equals(DistributionManager.State.GENERATED)) {
        quiz.setQuizStatus(Quiz.Status.AWAIT);
      } else if (state.equals(DistributionManager.State.ERROR)) {
        logger.error("Quiz's distribution generation failed: " + quiz);
        try {
          quiz.setQuizStatus(QuizError.DISTR_ERROR, "Distribution generation failed");
        } catch (QuizException e) {
          logger.error(e, e);
        }
      } else {
        quiz.setLastDistrStatusCheck(System.currentTimeMillis());
      }
    } catch (Exception e) {
      logger.error(e, e);
      throw new QuizException(e);
    }

  }

  private void exportStats(final Quiz quiz) throws QuizException {
    try {
      if (!quiz.isExported()) {
        quiz.setQuizStatus(Quiz.Status.EXPORTING);
        executor.execute(new Runnable() {
          public void run() {
            try {
              quiz.exportStats();         //already synchronized
              quiz.setQuizStatus(Quiz.Status.FINISHED);
              quiz.shutdown();
            } catch (QuizException e) {
              quiz.writeError(e);
              try {
                quiz.setQuizStatus(QuizError.UNKNOWN, "Error during export stats");
              } catch (QuizException ex) {
              }
              logger.error("Error creating result statistics for quiz:" + quiz, e);
            }
          }
        });
      } else {
        quiz.setQuizStatus(Quiz.Status.FINISHED);
      }
    } catch (Exception e) {
      logger.error(e, e);
      throw new QuizException(e);
    }
  }
}



