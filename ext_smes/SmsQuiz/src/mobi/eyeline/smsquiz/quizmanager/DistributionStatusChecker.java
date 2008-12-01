package mobi.eyeline.smsquiz.quizmanager;

import mobi.eyeline.smsquiz.distribution.DistributionManager;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import mobi.eyeline.smsquiz.quizmanager.quiz.QuizError;
import mobi.eyeline.smsquiz.quizmanager.quiz.Status;
import org.apache.log4j.Logger;

import java.util.Map;

/**
 * author: alkhal
 */
public class DistributionStatusChecker implements Runnable {

  private static final Logger logger = Logger.getLogger(DistributionStatusChecker.class);

  private Quizes quizes;

  private final DistributionManager dm;

  private final Map<String, Quiz> quizesMap;

  public DistributionStatusChecker(DistributionManager dm, Quizes quizes, Map<String, Quiz> quizesMap) {
    if ((dm == null) || (quizes == null) || (quizesMap == null)) {
      throw new NullPointerException("Some params are null");
    }
    this.dm = dm;
    this.quizes = quizes;
    this.quizesMap = quizesMap;
  }

  public void run() {
    logger.info("DistributionStatusChecker starts...");
    try {
      Quizes.Visitor visitor = new StatusVisitor();
      quizes.visit(visitor);
      logger.info("DistributionStatusChecker finished.");
    } catch (Exception e) {
      logger.error(e, e);
      e.printStackTrace();
    }
  }


  private class StatusVisitor implements Quizes.Visitor {

    public void visit(Quiz quiz) throws QuizException {
      try {
        if (quiz.isGenerated() || quiz.getQuizStatus().equals(Status.QuizStatus.FINISHED_ERROR)) {
          return;
        }
        String id = quiz.getDistrId();
        if (id == null) {
          logger.error("Distr id is null for quiz: " + quiz);
          return;
        }
        DistributionManager.State state = dm.getState(id);
        if (state.equals(DistributionManager.State.GENERATED)) {
          quiz.setGenerated(true);
          if (quiz.getQuizStatus().equals(Status.QuizStatus.GENERATION)) {
            if (!quiz.isActive()) {
              quiz.setQuizStatus(Status.QuizStatus.AWAIT);
            } else {
              quiz.setQuizStatus(Status.QuizStatus.ACTIVE);
            }
          }
        } else if (state.equals(DistributionManager.State.ERROR)) {
          logger.error("Quiz's distribution generation failed: " + quiz);
          if (quizesMap.get(quiz.getDestAddress()) == quiz) {
            quizesMap.remove(quiz.getDestAddress());
          }
          try {
            quiz.setError(QuizError.DISTR_ERROR, "Distribution generation failed");
          } catch (QuizException e) {
            logger.error(e, e);
          }
        }
      } catch (Exception e) {
        logger.error(e, e);
        throw new QuizException(e);
      }
    }
  }


}

